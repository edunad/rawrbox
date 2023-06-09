#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class TextureFlat : public TextureBase {
	private:
		std::vector<uint8_t> _pixels;
		bool _transparent = false;

	public:
		TextureFlat(const rawrbox::Vector2i& initsize, const rawrbox::Color& bgcol);
		[[nodiscard]] const bool hasTransparency() const override;

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------
	};
} // namespace rawrbox
