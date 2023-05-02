#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/utils/math.hpp>
#include <rawrbox/utils/time.hpp>

#include <stb/gif.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

namespace rawrBox {
	TextureGIF::TextureGIF(const std::string& fileName, bool useFallback) {
		this->_frames.clear();

		int frames_n = 0;
		int w = 0;
		int h = 0;
		int* delays = nullptr;

		// Need to find a way to not load it all to memory
		auto gifPixels = stbi_xload(fileName.c_str(), &w, &h, &frames_n, &delays);
		if (gifPixels == nullptr || delays == nullptr) {
			auto failure = stbi_failure_reason();

			if (useFallback) {
				this->_failedToLoad = true;
				fmt::print("[TextureGIF] Error loading gif image '{}' | Error: {} --- > Using fallback image\n", fileName, failure);
				return;
			} else {
				throw std::runtime_error(fmt::format("[TextureGIF] Error loading image: {}", failure));
			}
		}

		this->_size = {w, h};
		uint32_t framePixelCount = this->_size.x * this->_size.y * this->_channels;

		for (int i = 0; i < frames_n; i++) {
			// first push it, then allocate to prevent double copy of memory
			this->_frames.push_back({});

			GIFFrame& frame = this->_frames.back();
			frame.delay = delays[i]; // in ms
			frame.pixels.resize(framePixelCount);

			auto pixelsOffset = gifPixels + i * framePixelCount;
			std::copy(pixelsOffset, pixelsOffset + framePixelCount, frame.pixels.data());
		}

		stbi_image_free(gifPixels);
		stbi_image_free(delays);
	}

	void TextureGIF::update() {
		auto& frame = this->_frames[this->_currentFrame];
		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), bgfx::makeRef(frame.pixels.data(), static_cast<uint32_t>(frame.pixels.size())));
	}

	// ------ANIMATION
	void TextureGIF::step() {
		if (this->_failedToLoad || !bgfx::isValid(this->_handle)) return; // Not bound

		if (!this->_loop && this->_currentFrame >= this->_frames.size() - 1) return;
		if (this->_cooldown >= rawrBox::TimeUtils::curTime()) return;

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_currentFrame].delay * this->_speed) + rawrBox::TimeUtils::curTime(); // TODO: FIX SPEED
		this->_currentFrame = MathUtils::repeat<int>(this->_currentFrame + 1, 0, static_cast<int>(this->_frames.size()) - 1);

		this->update();
	}

	void TextureGIF::reset() {
		if (this->_failedToLoad || !bgfx::isValid(this->_handle)) return; // Not bound

		this->_cooldown = 0;
		this->_currentFrame = 0;
		this->update();
	}
	// --------------------

	// ------UTILS
	void TextureGIF::setLoop(bool loop) {
		this->_loop = loop;
	}

	void TextureGIF::setSpeed(float speed) {
		this->_speed = speed;
	}
	// --------------------

	// ------RENDER
	void TextureGIF::upload(bgfx::TextureFormat::Enum format) {
		if (this->_failedToLoad || bgfx::isValid(this->_handle)) return; // Failed texture is already bound, so skip it
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, format, 0 | this->_flags);

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureGIF] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-GIF-TEXTURE-{}", this->_handle.idx).c_str());
	}
	// --------------------
} // namespace rawrBox
