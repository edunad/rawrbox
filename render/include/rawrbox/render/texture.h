#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/color.hpp>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class Texture {
	private:
		const bgfx::Memory* _pixels;
		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;

		rawrBox::Vector2i _originalSize;
		rawrBox::Vector2i _size;

		int _index;
		int _channels = 4;

	public:
		Texture() = default;
		Texture(const Texture& t) = default;
		~Texture();

		Texture(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol = Colors::Transparent);
		Texture(const std::string& fileName);

#pragma region PIXEL-UTILS
		rawrBox::Color getPixel(unsigned int x, unsigned int y);
		rawrBox::Color getPixel(const rawrBox::Vector2i& pos);

		void setPixel(unsigned int x, unsigned int y, const rawrBox::Color& col);
		void setPixel(const rawrBox::Vector2i& pos, const rawrBox::Color& col);

		void resize(const rawrBox::Vector2i& newsize);
#pragma endregion

		void upload(int index = 0);
		bgfx::TextureHandle& getHandle();
	};
}
