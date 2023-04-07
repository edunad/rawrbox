#pragma once

#include <rawrbox/render/texture/base.h>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureFlat : public TextureBase {
	private:
		const bgfx::Memory* _pixels;

	public:
		TextureFlat(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol = Colors::Transparent);

		// ------RENDER
		virtual void upload() override;
		// --------------------
	};
}
