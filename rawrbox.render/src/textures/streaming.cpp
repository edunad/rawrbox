#include <rawrbox/math/utils/color.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/streaming.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureStreaming::TextureStreaming(const rawrbox::Vector2u& size) {
		this->setName("STREAMING");

		this->_data.size = size;
		this->_data.channels = 4U;
		this->_data.createFrame();
	}

	// UTILS ---
	void TextureStreaming::clear() {
		// We don't update the texture pixels, instead show MISSING_TEXTURE
		this->_hasData = false;
		this->_pendingUpdate = false;
	}

	void TextureStreaming::setImage(const rawrbox::ImageData& data) {
		if (data.frames.empty()) RAWRBOX_CRITICAL("Cannot set empty data");

		auto frame = data.pixels();
		if (frame.empty()) RAWRBOX_CRITICAL("Cannot set empty pixels");

		if (data.channels != this->_data.channels) frame = rawrbox::ColorUtils::setChannels(data.channels, this->_data.channels, data.size.x, data.size.y, frame);
		frame = rawrbox::TextureUtils::resize(data.size, frame, this->_data.size, this->_data.channels);

		std::memcpy(this->_data.pixels().data(), frame.data(), frame.size());
		this->_hasData = true;

		if (!this->_pendingUpdate) {
			this->_pendingUpdate = true;
			rawrbox::BindlessManager::registerUpdateTexture(*this);
		}
	}

	bool TextureStreaming::hasData() const {
		return this->_hasData;
	}
	// --------

	void TextureStreaming::update() {
		if (!this->_pendingUpdate || this->_tex == nullptr) return;
		if (this->_data.empty()) RAWRBOX_CRITICAL("Cannot update empty data");

		auto* context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_data.size.x;
		UpdateBox.MaxY = this->_data.size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_data.size.x * this->_data.channels;
		SubresData.pData = this->_data.pixels().data();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		//  ------------

		this->_pendingUpdate = false;
	}

	bool TextureStreaming::requiresUpdate() const {
		return this->_pendingUpdate;
	}

	uint32_t TextureStreaming::getTextureID() const {
		if (!this->hasData()) return rawrbox::MISSING_TEXTURE->getTextureID();
		return rawrbox::TextureBase::getTextureID();
	}

	void TextureStreaming::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		rawrbox::TextureBase::upload(format, true);
	}
} // namespace rawrbox
