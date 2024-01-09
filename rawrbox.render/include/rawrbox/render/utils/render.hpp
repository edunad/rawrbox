#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class RenderUtils {
		static Diligent::IPipelineState* _pipe;
		static void init();

	public:
		static void renderQUAD(rawrbox::TextureBase* texture);
	};
} // namespace rawrbox
