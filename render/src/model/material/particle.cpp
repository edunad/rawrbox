
#include <rawrbox/render/model/material/particle.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader particle_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_particle),
    BGFX_EMBEDDED_SHADER(fs_particle),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialParticle::upload() {
		this->buildShader(particle_shaders);
	}
} // namespace rawrbox
