#include <rawrbox/render_temp/materials/decal.hpp>
#include <rawrbox/render_temp/renderers/base.hpp>
#include <rawrbox/render_temp/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_decal_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_decal_base),
    BGFX_EMBEDDED_SHADER(fs_decal_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	void MaterialDecal::setupUniforms() {
		rawrbox::MaterialInstancedLit::setupUniforms();

		this->registerUniform("s_depth", bgfx::UniformType::Sampler);
		this->registerUniform("s_mask", bgfx::UniformType::Sampler);
		this->registerUniform("u_decalSettings", bgfx::UniformType::Vec4);
	}

	void MaterialDecal::upload() {
		this->setupUniforms();
		rawrbox::RenderUtils::buildShader(model_decal_shaders, this->_program);
	}

	void MaterialDecal::process(const rawrbox::Mesh& mesh) {
		if (rawrbox::RENDERER == nullptr) return;
		rawrbox::MaterialInstancedLit::process(mesh);

		bgfx::setTexture(rawrbox::SAMPLE_DEPTH, this->getUniform("s_depth"), rawrbox::RENDERER->getDepth());
		bgfx::setTexture(rawrbox::SAMPLE_MASK, this->getUniform("s_mask"), rawrbox::RENDERER->getMask());

		std::array<float, 4> settings = {rawrbox::RENDERER_DEBUG == rawrbox::DEBUG_DECALS ? 1.F : 0.F};
		bgfx::setUniform(this->getUniform("u_decalSettings"), settings.data());
	}
} // namespace rawrbox
