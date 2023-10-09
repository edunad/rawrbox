#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render_temp/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <vector>

namespace rawrbox {
	class TextureFlat : public TextureBase {
	private:
		std::vector<uint8_t> _pixels;
		bool _transparent = false;

	public:
		TextureFlat(const rawrbox::Vector2i& initsize, const rawrbox::Color& bgcol);
		[[nodiscard]] bool hasTransparency() const override;

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrbox
