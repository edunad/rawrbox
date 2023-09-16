#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/animated.hpp>
#include <rawrbox/utils/time.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// NOLINTBEGIN(modernize-pass-by-value)
	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		this->internalLoad({}, useFallback);
	}

	TextureAnimatedBase::TextureAnimatedBase(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) {
		this->internalLoad(buffer, useFallback);
	}
	// NOLINTEND(modernize-pass-by-value)

	void TextureAnimatedBase::internalLoad(const std::vector<uint8_t>& /*_buffer*/, bool /*_useFallback*/) {}
	bool TextureAnimatedBase::hasTransparency() const {
		return this->_channels == 4 && this->_transparent;
	}

	void TextureAnimatedBase::internalUpdate() {
		auto& frame = this->_frames[this->_currentFrame];
		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), bgfx::makeRef(frame.pixels.data(), static_cast<uint32_t>(frame.pixels.size())));
	}

	// ANIMATION ------
	void TextureAnimatedBase::update() {
		if (this->_failedToLoad || !bgfx::isValid(this->_handle)) return; // Not bound
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
		if (this->_failedToLoad || !bgfx::isValid(this->_handle)) return; // Not bound

		this->_cooldown = 0;
		this->_pause = false;
		this->_currentFrame = 0;
		this->update();
	}
	// --------------------

	// UTILS ------
	void TextureAnimatedBase::setPaused(bool paused) {
		this->_pause = paused;
	}

	void TextureAnimatedBase::setLoop(bool loop) {
		this->_loop = loop;
	}

	void TextureAnimatedBase::setSpeed(float speed) {
		this->_speed = speed;
	}
	// --------------------

	// RENDER ------
	void TextureAnimatedBase::upload(bgfx::TextureFormat::Enum format) {
		if (this->_failedToLoad || bgfx::isValid(this->_handle)) return; // Failed texture is already bound, so skip it

		// Try to determine
		if (format == bgfx::TextureFormat::Count) {
			switch (this->_channels) {
				case 1:
					format = bgfx::TextureFormat::R8;
					break;
				case 2:
					format = bgfx::TextureFormat::RG8;
					break;
				case 3:
					format = bgfx::TextureFormat::RGB8;
					break;
				default:
				case 4:
					format = bgfx::TextureFormat::RGBA8;
					break;
			}
		}

		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, format, 0 | this->_flags);

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureAnimated] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-ANIMATED-TEXTURE-{}", this->_handle.idx).c_str());
	}
	// --------------------
} // namespace rawrbox
