#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class RenderUtils {
	protected:
		static Diligent::IPipelineState* _pipe;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void init();
		static void renderQUAD(const rawrbox::TextureBase& texture);
	};
} // namespace rawrbox
