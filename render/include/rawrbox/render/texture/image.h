#pragma once

#include <rawrbox/render/texture/base.h>

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <string>
#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureImage : public TextureBase {
	private:
		const bgfx::Memory* _pixels;

	public:
		TextureImage(const std::string& fileName);

		// ------ PIXEL-UTILS
		virtual rawrBox::Color getPixel(unsigned int x, unsigned int y);
		virtual rawrBox::Color getPixel(const rawrBox::Vector2i& pos);

		virtual void setPixel(unsigned int x, unsigned int y, const rawrBox::Color& col);
		virtual void setPixel(const rawrBox::Vector2i& pos, const rawrBox::Color& col);

		virtual void resize(const rawrBox::Vector2i& newsize);
		// --------------------

		virtual void upload() override;
	};
} // namespace rawrBox
