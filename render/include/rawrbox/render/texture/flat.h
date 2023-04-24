#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureFlat : public TextureBase {
	private:
		std::vector<uint8_t> _pixels;

	public:
		TextureFlat(const rawrBox::Vector2i& initsize, const rawrBox::Color& bgcol = rawrBox::Colors::White);

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrBox
