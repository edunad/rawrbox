#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/color.hpp>

#include <bgfx/bgfx.h>

namespace rawrBOX {
	class Texture {
	private:
		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _texColor = BGFX_INVALID_HANDLE;

		rawrBOX::Vector2i _originalSize;
		rawrBOX::Vector2i _size;
		std::vector<rawrBOX::Color> _pixels;

		int _index;

	public:
		Texture() = default;
		Texture(const Texture& t) = default;
		~Texture();

		Texture(const rawrBOX::Vector2i& initsize, const rawrBOX::Color& bgcol = Colors::Transparent);
		Texture(const std::string& fileName);

#pragma region PIXEL-UTILS
		rawrBOX::Color& getPixel(unsigned int x, unsigned int y);
		const rawrBOX::Color& getPixel(unsigned int x, unsigned int y) const;
		rawrBOX::Color& getPixel(const rawrBOX::Vector2i& pos);
		const rawrBOX::Color& getPixel(const rawrBOX::Vector2i& pos) const;

		void setPixel(const rawrBOX::Vector2i& pos, const rawrBOX::Color& col);
		void setPixels(const rawrBOX::Vector2i& size_, const std::vector<rawrBOX::Color>& data);

		void resize(const rawrBOX::Vector2i& newsize);
#pragma endregion

		void bind(int index = 0);
		void use();
	};
}
