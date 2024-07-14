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

		this->_checker_1 = rawrbox::TextureUtils::generateCheckboard(this->_size, rawrbox::Color::RGBHex(0x343434), rawrbox::Color::RGBHex(0x666666), 8);
		this->_checker_2 = rawrbox::TextureUtils::generateCheckboard(this->_size, rawrbox::Color::RGBHex(0x666666), rawrbox::Color::RGBHex(0x343434), 8);
	}

	void TextureStreaming::setPixelData(const std::vector<uint8_t>& data) {
		this->_pixels = data;

		if (!this->_pendingUpdate) {
			this->_pendingUpdate = true;
			rawrbox::BindlessManager::registerUpdateTexture(*this);
		}
	}

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

		if (this->_pixels.empty()) {
			SubresData.pData = this->_tick > 15 ? this->_checker_1.data() : this->_checker_2.data();
			this->_tick = rawrbox::MathUtils::repeat(this->_tick + 1, 0, 30);
		} else {
			SubresData.pData = this->_pixels.data();
			this->_pendingUpdate = false;
		}

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		//  ------------
	}

	bool TextureStreaming::requiresUpdate() const {
		return this->_pendingUpdate;
	}

	void TextureStreaming::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		rawrbox::TextureBase::upload(format, true);
		this->_pixels.clear();
	}
} // namespace rawrbox
