#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader particle_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_particle_unlit),
    BGFX_EMBEDDED_SHADER(fs_particle_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	class MaterialParticleUnlit : public rawrbox::MaterialBase {
	public:
		using vertexBufferType = rawrbox::VertexBlendData;
		MaterialParticleUnlit() = default;

		void upload() {
			this->buildShader(particle_unlit_shaders, "particle_unlit");
		}
	};
} // namespace rawrbox
