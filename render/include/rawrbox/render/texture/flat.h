#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureFlat : public TextureBase {
	private:
		const bgfx::Memory* _pixels;
		bgfx::TextureFormat::Enum _format;

	public:
		TextureFlat(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol = Colors::Transparent, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8);

		// ------RENDER
		virtual void upload() override;
		// --------------------
	};
} // namespace rawrBox
