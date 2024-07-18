#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureFlat : public rawrbox::TextureBase {
	private:
	public:
		TextureFlat(const rawrbox::Vector2u& size, const rawrbox::Color& bgcol);
		[[nodiscard]] bool hasTransparency() const override;
	};
} // namespace rawrbox
