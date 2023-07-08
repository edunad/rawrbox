
#include <rawrbox/render/materials/particle.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader particle_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_particle),
    BGFX_EMBEDDED_SHADER(fs_particle),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialParticle::upload() {
		rawrbox::RenderUtils::buildShader(particle_shaders, this->_program);
	}
} // namespace rawrbox
