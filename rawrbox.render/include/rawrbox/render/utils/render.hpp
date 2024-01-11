#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class RenderUtils {
		static Diligent::IPipelineState* _pipe;

	public:
		static void init();
		static void renderQUAD(const rawrbox::TextureBase& texture);
	};
} // namespace rawrbox
