#pragma once

#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>
#include <string>
#include <vector>

namespace rawrBox {
	struct GIFFrame {
		int delay;
		const bgfx::Memory* pixels;
	};

	class AnimatedTexture {
	private:
		std::vector<GIFFrame> _pixels;

		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;
		rawrBox::Vector2i _size;

		int _index;
		int _channels = 4;

		int _currentFrame = 0;
		bool _loop = true;

		int64_t _cooldown = 0;
	public:
		AnimatedTexture() = default;
		AnimatedTexture(const AnimatedTexture& t) = default;
		~AnimatedTexture();

		AnimatedTexture(const std::string& fileName);

		void upload(int index = 0);
		void step();
		void reset();
		void setLoop(bool loop);
		void update();

		bgfx::TextureHandle& getHandle();
	};
}
