#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureFlat : public rawrbox::TextureBase {
	private:
		bool _transparent = false;

	public:
		TextureFlat(const rawrbox::Vector2i& initsize, const rawrbox::Color& bgcol);
		[[nodiscard]] bool hasTransparency() const override;
	};
} // namespace rawrbox
