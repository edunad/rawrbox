#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader particle_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_particle),
    BGFX_EMBEDDED_SHADER(fs_particle),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	class MaterialParticle : public rawrbox::MaterialBase {
	public:
		using vertexBufferType = rawrbox::VertexBlendData;
		MaterialParticle() = default;

		void upload() {
			this->buildShader(particle_shaders, "particle");
		}
	};
} // namespace rawrbox
