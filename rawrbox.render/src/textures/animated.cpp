#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/animated.hpp>
#include <rawrbox/utils/time.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, bool /*useFallback*/) : _filePath(filePath) {}
	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, const std::vector<uint8_t>& /*buffer*/, bool /*useFallback*/) : _filePath(filePath) {}
	// NOLINTEND(modernize-pass-by-value)

	void TextureAnimatedBase::internalLoad(const std::vector<uint8_t>& /*_buffer*/, bool /*_useFallback*/) { throw std::runtime_error("[RawrBox-TextureAnimatedBase] Not implemented"); }
	void TextureAnimatedBase::internalUpdate() {
		auto context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_size.x;
		UpdateBox.MaxY = this->_size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_size.x * this->_channels;
		SubresData.pData = this->_frames.empty() ? this->_pixels.data() : this->_frames[this->_currentFrame].pixels.data();

		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	// ANIMATION ------
	void TextureAnimatedBase::update() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound
		if (this->_pause || this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		if (static_cast<size_t>(this->_currentFrame) >= this->_frames.size()) {
			this->onEnd();

			if (!this->_loop) {
				this->setPaused(true);
				return;
			} else {
				this->_currentFrame = 0;
			}
		}

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_currentFrame].delay / this->_speed) + rawrbox::TimeUtils::curtime();
		this->internalUpdate();

		this->_currentFrame++;
	}

	void TextureAnimatedBase::reset() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound

		this->_cooldown = 0;
		this->_pause = false;
		this->_currentFrame = 0;
		this->update();
	}
	// --------------------

	// UTILS ------
	bool TextureAnimatedBase::getPaused() { return this->_pause; }
	void TextureAnimatedBase::setPaused(bool paused) {
		this->_pause = paused;
	}

	bool TextureAnimatedBase::getLoop() { return this->_loop; }
	void TextureAnimatedBase::setLoop(bool loop) {
		this->_loop = loop;
	}

	float TextureAnimatedBase::getSpeed() { return this->_speed; }
	void TextureAnimatedBase::setSpeed(float speed) {
		this->_speed = speed;
	}
	// --------------------

	// RENDER ------
	void TextureAnimatedBase::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it
		rawrbox::TextureBase::upload(format, true);
	}
	// --------------------

	bool TextureAnimatedBase::requiresUpdate() const { return true; }
} // namespace rawrbox
