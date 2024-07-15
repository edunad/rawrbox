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

		this->_size = size;
		this->_channels = 4U;
	}

	// UTILS ---
	void TextureStreaming::setImage(const rawrbox::ImageData& data) {
		if (data.frames.empty()) throw _logger->error("Cannot set empty data");

		const auto& frame = data.frames.front();
		if (frame.pixels.empty()) throw _logger->error("Cannot set empty pixels");

		this->_pixels = rawrbox::TextureUtils::resize(data.size, frame.pixels, this->_size, data.channels);
		if (data.channels != this->_channels) this->_pixels = rawrbox::ColorUtils::setChannels(data.channels, this->_channels, this->_size.x, this->_size.y, this->_pixels);

		this->_hasData = true;
		if (!this->_pendingUpdate) {
			this->_pendingUpdate = true;
			rawrbox::BindlessManager::registerUpdateTexture(*this);
		}
	}

	bool TextureStreaming::hasData() const { return this->_hasData; }
	// --------

	void TextureStreaming::update() {
		if (!this->_pendingUpdate || this->_tex == nullptr) return;
		auto* context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_size.x;
		UpdateBox.MaxY = this->_size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_size.x * this->_channels;
		SubresData.pData = this->_pixels.data();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		//  ------------

		this->_pendingUpdate = false;
	}

	bool TextureStreaming::requiresUpdate() const {
		return this->_pendingUpdate;
	}

	void TextureStreaming::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		rawrbox::TextureBase::upload(format, true);
		this->_pixels.clear();
	}
} // namespace rawrbox
