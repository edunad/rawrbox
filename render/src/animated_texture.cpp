#pragma once

#include <rawrbox/render/animated_texture.h>
#include <rawrbox/utils/math.hpp>
#include <rawrbox/utils/time.h>

#include <fmt/format.h>

#include <bgfx/bgfx.h>
#include <stb/gif.hpp>


namespace rawrBox {
	AnimatedTexture::~AnimatedTexture() {
		bgfx::destroy(this->_handle);
	}

	AnimatedTexture::AnimatedTexture(const std::string& fileName) {
		this->_pixels.clear();

		int frames_n = 0;
		int w = 0;
		int h = 0;
		int* delays = nullptr;

		// Need to find a way to not load it all to memory
		const auto packed_gif_frames = stbi_xload(
			fileName.c_str(),
			&w,
			&h,
			&frames_n,
			&delays
		);

		this->_size = {w, h};

		if (packed_gif_frames == nullptr || delays == nullptr) throw std::runtime_error("Invalid image");

		const auto single_image_bytes = w * h * 4; // * channels
		uint32_t byteSize = static_cast<uint32_t>(this->_size.x * this->_size.y) * this->_channels;

		for (int i = 0; i < frames_n; ++i) {
			GIFFrame frame;
			frame.delay = delays[i]; // in ms
			frame.pixels = bgfx::alloc(byteSize);
			std::memcpy(frame.pixels->data, packed_gif_frames + i * single_image_bytes, byteSize);

			this->_pixels.push_back(frame);
		}

		stbi_image_free(reinterpret_cast<void*>(packed_gif_frames));
		stbi_image_free(reinterpret_cast<void*>(delays));
	}

	void AnimatedTexture::setLoop(bool loop) {
		this->_loop = loop;
	}

	void AnimatedTexture::step() {
		if(!bgfx::isValid(this->_handle)) return; // Already bound

		if(!this->_loop && this->_currentFrame >= this->_pixels.size() - 1) return;
		if(this->_cooldown >= rawrBox::TimeUtils::curTime()) return;

		this->_cooldown = static_cast<int64_t>(this->_pixels[this->_currentFrame].delay) + rawrBox::TimeUtils::curTime();
		this->_currentFrame = MathUtils::repeat<int>(this->_currentFrame + 1, 0, static_cast<int>(this->_pixels.size()) - 1);

		this->update();
	}

	void AnimatedTexture::reset() {
		if(!bgfx::isValid(this->_handle)) return; // Already bound

		this->_cooldown = 0;
		this->_currentFrame = 0;
		this->update();
	}

	void AnimatedTexture::update() {
		uint32_t byteSize = static_cast<uint32_t>(this->_size.x * this->_size.y) * this->_channels;

		const bgfx::Memory* mem = bgfx::copy(this->_pixels[this->_currentFrame].pixels, byteSize);
		bgfx::updateTexture2D(this->_handle, 0, 0, 0, 0, static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), mem);
	}

	void AnimatedTexture::upload(int index) {
		if(bgfx::isValid(this->_handle)) return; // Already bound

		this->_index = index;
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 0, bgfx::TextureFormat::RGBA8,
              BGFX_SAMPLER_U_BORDER
            | BGFX_SAMPLER_V_BORDER
            | BGFX_SAMPLER_MIN_POINT
            | BGFX_SAMPLER_MAG_POINT);

		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[Texture] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-ANIMATED-TEXTURE-{}-{}", this->_index, this->_handle.idx).c_str());
	}

	bgfx::TextureHandle& AnimatedTexture::getHandle() {
		return this->_handle;
	}
}
