#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/utils/time.hpp>

#include <stb/gif.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

namespace rawrbox {

	// NOLINTBEGIN(modernize-pass-by-value)
	TextureGIF::TextureGIF(const std::filesystem::path& filePath, bool useFallback) : _filePath(filePath) {
		this->internalLoad({}, useFallback);
	}

	TextureGIF::TextureGIF(const std::filesystem::path& filePath, const std::vector<uint8_t>& buffer, bool useFallback) : _filePath(filePath) {
		this->internalLoad(buffer, useFallback);
	}
	// NOLINTEND(modernize-pass-by-value)

	void TextureGIF::internalLoad(const std::vector<uint8_t>& buffer, bool useFallback) {
		this->_frames.clear();
		this->_channels = 4; // Force 4 channels on GIFS

		int frames_n = 0;
		int* delays = nullptr;
		uint8_t* gifPixels = nullptr;

		// Need to find a way to not load it all to memory
		if (buffer.empty()) {
			gifPixels = stbi_xload_file(this->_filePath.generic_string().c_str(), &this->_size.x, &this->_size.y, &frames_n, &delays);
		} else {
			gifPixels = stbi_xload_mem(buffer.data(), static_cast<int>(buffer.size()) * sizeof(uint8_t), &this->_size.x, &this->_size.y, &frames_n, &delays);
		}

		if (gifPixels == nullptr || delays == nullptr) {
			auto failure = stbi_failure_reason();

			if (useFallback) {
				this->_failedToLoad = true;
				fmt::print("[TextureGIF] Error loading gif image '{}' | Error: {} --- > Using fallback image\n", this->_filePath.generic_string(), failure);
				return;
			} else {
				throw std::runtime_error(fmt::format("[TextureGIF] Error loading image: {}", failure));
			}
		}

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

		// Check for transparency ----
		if (this->_channels == 4 && !this->_frames.empty()) {
			auto& frame1 = this->_frames.back().pixels;
			for (size_t i = 0; i < frame1.size(); i += this->_channels) {
				if (frame1[i + 3] == 1.F) continue;
				_transparent = true;
				break;
			}
		}
		// ---------------------------

		stbi_image_free(gifPixels);
		stbi_image_free(delays);
	}

	const bool TextureGIF::hasTransparency() const {
		return this->_channels == 4 && this->_transparent;
	}

	void TextureGIF::update() {
		auto& frame = this->_frames[this->_currentFrame];
		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), bgfx::makeRef(frame.pixels.data(), static_cast<uint32_t>(frame.pixels.size())));
	}

	// ------ANIMATION
	void TextureGIF::step() {
		if (this->_failedToLoad || !bgfx::isValid(this->_handle)) return; // Not bound

		if (!this->_loop && this->_currentFrame >= this->_frames.size() - 1) return;
		if (this->_cooldown >= rawrbox::TimeUtils::curtime()) return;

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_currentFrame].delay * this->_speed) + rawrbox::TimeUtils::curtime(); // TODO: FIX SPEED
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
} // namespace rawrbox
