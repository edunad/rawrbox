#include <rawrbox/render/materials/decal.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/render.hpp>

#include "bx/math.h"
/*
// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_decal_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_decal_base),
    BGFX_EMBEDDED_SHADER(fs_decal_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)
*/
namespace rawrbox {
	MaterialDecal::~MaterialDecal() {
		RAWRBOX_DESTROY(this->_s_depth);
		RAWRBOX_DESTROY(this->_s_mask);

		RAWRBOX_DESTROY(this->_u_decalSettings);
	}

	void MaterialDecal::registerUniforms() {
		rawrbox::MaterialInstancedLit::registerUniforms();

		this->_s_depth = bgfx::createUniform("s_depth", bgfx::UniformType::Sampler);
		this->_s_mask = bgfx::createUniform("s_mask", bgfx::UniformType::Sampler);
		this->_u_decalSettings = bgfx::createUniform("u_decalSettings", bgfx::UniformType::Vec4);
	}

	void MaterialDecal::upload() {
		// rawrbox::RenderUtils::buildShader(model_decal_shaders, this->_program);
	}

	void MaterialDecal::process(const rawrbox::Mesh& mesh) {
		if (rawrbox::RENDERER == nullptr) return;
		/*
				rawrbox::MaterialInstancedLit::process(mesh);

				bgfx::setTexture(rawrbox::SAMPLE_DEPTH, this->_s_depth, rawrbox::RENDERER->getDepth());
				bgfx::setTexture(rawrbox::SAMPLE_MASK, this->_s_mask, rawrbox::RENDERER->getMask());

				std::array<float, 4> settings = {rawrbox::RENDERER_DEBUG == rawrbox::DEBUG_DECALS ? 1.F : 0.F};
				bgfx::setUniform(this->_u_decalSettings, settings.data());*/
	}
} // namespace rawrbox
