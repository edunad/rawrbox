#include <rawrbox/render/texture/gif.h>
#include <rawrbox/utils/math.hpp>
#include <rawrbox/utils/time.h>

#include <fmt/format.h>

#include <bgfx/bgfx.h>
#include <stb/gif.hpp>

namespace rawrBox {
	TextureGIF::TextureGIF(const std::string& fileName) {
		this->_frames.clear();

		int frames_n = 0;
		int w = 0;
		int h = 0;
		int* delays = nullptr;

		// Need to find a way to not load it all to memory
		auto gifPixels = stbi_xload(
			fileName.c_str(),
			&w,
			&h,
			&frames_n,
			&delays
		);

		this->_size = {w, h};

		if (gifPixels == nullptr || delays == nullptr) throw std::runtime_error("Invalid image");

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
		if(!bgfx::isValid(this->_handle)) return; // Not bound

		if(!this->_loop && this->_currentFrame >= this->_frames.size() - 1) return;
		if(this->_cooldown >= rawrBox::TimeUtils::curTime()) return;

		this->_cooldown = static_cast<int64_t>(this->_frames[this->_currentFrame].delay * this->_speed) + rawrBox::TimeUtils::curTime(); // TODO: FIX SPEED
		this->_currentFrame = MathUtils::repeat<int>(this->_currentFrame + 1, 0, static_cast<int>(this->_frames.size()) - 1);

		this->update();
	}

	void TextureGIF::reset() {
		if(!bgfx::isValid(this->_handle)) return; // Not bound

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
	void TextureGIF::upload() {
		if(bgfx::isValid(this->_handle)) return; // Already bound
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, bgfx::TextureFormat::RGBA8,
              BGFX_SAMPLER_U_BORDER
            | BGFX_SAMPLER_V_BORDER
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT);

		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureGIF] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-GIF-TEXTURE-{}", this->_handle.idx).c_str());
	}
	// --------------------
}
