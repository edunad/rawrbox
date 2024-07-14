#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureMissing : public rawrbox::TextureBase {
	public:
		TextureMissing(const rawrbox::Vector2u& size = {2, 2});
	};
} // namespace rawrbox
