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

	void TextureAnimatedBase::internalLoad(const std::vector<uint8_t>& /*_buffer*/, bool /*_useFallback*/) { CRITICAL_RAWRBOX("Not implemented"); }

	// ANIMATION ------
	void TextureAnimatedBase::update() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound
		if (this->_pause || this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		if (static_cast<size_t>(this->_slice) >= this->_data.total()) {
			this->onEnd();

			if (!this->_loop) {
				this->setPaused(true);
				return;
			}

			this->_slice = 0;
		}

		this->_cooldown = static_cast<int64_t>(this->_data.frames[this->_slice].delay / this->_speed) + rawrbox::TimeUtils::curtime();
		this->_slice++;
	}

	void TextureAnimatedBase::reset() {
		if (this->_failedToLoad || this->_handle == nullptr) return; // Not bound

		this->_pause = false;
		this->_cooldown = 0;
		this->_slice = 0;
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

	uint32_t TextureAnimatedBase::total() const { return static_cast<uint32_t>(this->_data.total()); }
	// --------------------

	bool TextureAnimatedBase::requiresUpdate() const { return true; }
} // namespace rawrbox
