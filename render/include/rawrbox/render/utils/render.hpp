#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/shader_defines.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class RenderUtils {
	public:
		// NOLINTBEGIN(*)
		static void buildShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		static void buildComputeShader(const bgfx::EmbeddedShader shaders[], bgfx::ProgramHandle& program);
		// NOLINTEND(*)
	};
} // namespace rawrbox
