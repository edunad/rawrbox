#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/animated.hpp>
#include <rawrbox/utils/time.hpp>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, bool /*useFallback*/) : _filePath(filePath) {}
	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, const std::vector<uint8_t>& /*buffer*/, bool /*useFallback*/) : _filePath(filePath) {}
	// NOLINTEND(modernize-pass-by-value)

	void TextureAnimatedBase::internalLoad(const std::vector<uint8_t>& /*_buffer*/, bool /*_useFallback*/) { throw this->_logger->error("Not implemented"); }
	void TextureAnimatedBase::internalUpdate() {
		auto* context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_size.x;
		UpdateBox.MaxY = this->_size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_size.x * this->_channels;
		SubresData.pData = this->_frames.empty() ? this->_pixels.data() : this->_frames[this->_slice].pixels.data();

		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
	}

	// ANIMATION ------
	void TextureAnimatedBase::update() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound
		if (this->_pause || this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		if (static_cast<size_t>(this->_slice) >= this->_frames.size()) {
			this->onEnd();

			if (!this->_loop) {
				this->setPaused(true);
				return;
			}

			this->_slice = 0;
		}

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_slice].delay / this->_speed) + rawrbox::TimeUtils::curtime();
		if (!this->_textureArray) this->internalUpdate(); // Handled by shader / slice instead

		this->_slice++;
	}

	void TextureAnimatedBase::reset() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound

		this->_pause = false;
		this->_cooldown = 0;
		this->_slice = 0;

		if (!this->_textureArray) this->internalUpdate(); // Handled by shader instead
	}
	// --------------------

	// UTILS ------
	bool TextureAnimatedBase::getPaused() const { return this->_pause; }
	void TextureAnimatedBase::setPaused(bool paused) {
		this->_pause = paused;
	}

	bool TextureAnimatedBase::getLoop() const { return this->_loop; }
	void TextureAnimatedBase::setLoop(bool loop) {
		this->_loop = loop;
	}

	float TextureAnimatedBase::getSpeed() const { return this->_speed; }
	void TextureAnimatedBase::setSpeed(float speed) {
		this->_speed = speed;
	}

	uint32_t TextureAnimatedBase::total() const { return static_cast<uint32_t>(this->_frames.size()); }
	uint32_t TextureAnimatedBase::getSlice() const {
		return this->_textureArray ? this->_slice : 0;
	}
	// --------------------

	// RENDER ------
	void TextureAnimatedBase::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it

		this->_textureArray = this->_size.x <= 1024 && this->_size.y <= 1024 && this->total() <= 256;
		if (!this->_textureArray) {
			rawrbox::TextureBase::upload(format, true);
			return;
		}

		// Try to determine texture format
		this->tryGetFormatChannels(format, this->_channels);
		// --------------------------------

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_size.x;
		desc.Height = this->_size.y;
		desc.MipLevels = 1;
		desc.Format = format;
		// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
		desc.ArraySize = static_cast<uint32_t>(this->_frames.size());
		// NOLINTEND(cppcoreguidelines-pro-type-union-access)
		desc.Name = this->_name.c_str();

		// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
		std::vector<Diligent::TextureSubResData> subresData(desc.ArraySize);
		// NOLINTEND(cppcoreguidelines-pro-type-union-access)

		for (uint32_t slice = 0; slice < subresData.size(); slice++) {
			auto& res = subresData[slice];

			res.pData = this->_frames[slice].pixels.data();
			res.Stride = desc.Width * this->_channels;
		}

		Diligent::TextureData data;
		data.pSubResources = subresData.data();
		data.NumSubresources = static_cast<uint32_t>(subresData.size());

		rawrbox::RENDERER->device()->CreateTexture(desc, &data, &this->_tex);
		if (this->_tex == nullptr) throw this->_logger->error("Failed to create texture '{}'", this->_name);

		// Get handles --
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		// -------

		rawrbox::runOnRenderThread([this]() {
			rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			rawrbox::BindlessManager::registerTexture(*this);
		});
	}
	// --------------------

	bool TextureAnimatedBase::requiresUpdate() const { return true; }
} // namespace rawrbox
