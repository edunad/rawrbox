#include <rawrbox/render/materials/decal.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/render.hpp>

#include "bx/math.h"

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_decal_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_decal_base),
    BGFX_EMBEDDED_SHADER(fs_decal_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialDecal::~MaterialDecal() {
		RAWRBOX_DESTROY(this->_s_depth);
	}

	void MaterialDecal::registerUniforms() {
		rawrbox::MaterialInstanced::registerUniforms();
		this->_s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);
	}

	void MaterialDecal::upload() {
		rawrbox::RenderUtils::buildShader(model_decal_shaders, this->_program);
	}

	void MaterialDecal::process(const rawrbox::Mesh& mesh) {
		if (rawrbox::RENDERER == nullptr) return;

		rawrbox::MaterialInstanced::process(mesh);
		bgfx::setTexture(rawrbox::SAMPLE_DEPTH, this->_s_depth, rawrbox::RENDERER->getDepth());
	}
} // namespace rawrbox
