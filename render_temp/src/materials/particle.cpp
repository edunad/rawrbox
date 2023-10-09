
#include <rawrbox/render_temp/materials/particle.hpp>
#include <rawrbox/render_temp/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader particle_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_particle),
    BGFX_EMBEDDED_SHADER(fs_particle),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	uint32_t MaterialParticle::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::PARTICLE;
	}

	void MaterialParticle::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(particle_shaders, this->_program);
	}
} // namespace rawrbox
