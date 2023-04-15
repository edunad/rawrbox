#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

#include <string>
#include <vector>

namespace rawrBox {
	struct GIFFrame {
		int delay;
		std::vector<uint8_t> pixels;
	};

	class TextureGIF : public TextureBase {
	private:
		std::vector<GIFFrame> _frames;

		int _currentFrame = 0;
		bool _loop = true;
		int64_t _cooldown = 0;
		float _speed = 1.f;

		void update();

	public:
		TextureGIF(const std::string& fileName);

		// ------ANIMATION
		virtual void step();
		virtual void reset();
		// --------------------

		// ------UTILS
		virtual void setLoop(bool loop);
		virtual void setSpeed(float speed);
		// --------------------

		// ------RENDER
		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrBox
