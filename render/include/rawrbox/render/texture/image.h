#pragma once

#include <rawrbox/render/texture/base.h>

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <string>
#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureImage : public TextureBase {
	private:
		std::vector<unsigned char> _pixels;

	public:
		TextureImage(const std::string& fileName);

		// ------ PIXEL-UTILS
		virtual rawrBox::Colori getPixel(unsigned int x, unsigned int y);
		virtual rawrBox::Colori getPixel(const rawrBox::Vector2i& pos);

		virtual void setPixel(unsigned int x, unsigned int y, const rawrBox::Colori& col);
		virtual void setPixel(const rawrBox::Vector2i& pos, const rawrBox::Colori& col);

		virtual void resize(const rawrBox::Vector2i& newsize);
		// --------------------

		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
	};
} // namespace rawrBox
