
#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader clustered_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER(fs_clustered_unlit_base),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(this->_program);

		RAWRBOX_DESTROY(this->_s_albedo);
		RAWRBOX_DESTROY(this->_s_displacement);

		// Uniforms -----
		RAWRBOX_DESTROY(this->_u_colorOffset);
		RAWRBOX_DESTROY(this->_u_mesh_pos);
		RAWRBOX_DESTROY(this->_u_data);
	}

	void MaterialBase::registerUniforms() {
		this->_s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler);
		this->_s_displacement = bgfx::createUniform("s_displacement", bgfx::UniformType::Sampler);

		this->_u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);

		this->_u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4);
		this->_u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4, MAX_DATA);
	}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->_s_albedo, mesh.texture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->_s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.displacementTexture != nullptr && mesh.displacementTexture->valid()) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_DISPLACEMENT, this->_s_displacement, mesh.displacementTexture->getHandle());
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_DISPLACEMENT, this->_s_displacement, rawrbox::BLACK_TEXTURE->getHandle());
		}

		// Color override
		bgfx::setUniform(this->_u_colorOffset, mesh.color.data().data());
		// -------

		// Mesh pos
		std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
		bgfx::setUniform(this->_u_mesh_pos, offset.data());
		// -------

		// Pass "special" data ---
		std::array<std::array<float, 4>, MAX_DATA> data = {std::array<float, 4>{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
		if (mesh.hasData("billboard_mode")) {
			data[0] = mesh.getData("billboard_mode").data();
		}

		if (mesh.hasData("vertex_snap")) {
			data[1] = mesh.getData("vertex_snap").data();
		}

		if (mesh.hasData("displacement_strength")) {
			data[2] = mesh.getData("displacement_strength").data();
		}

		if (mesh.hasData("mask")) {
			data[3] = mesh.getData("mask").data();
		}

		bgfx::setUniform(this->_u_data, data.front().data(), MAX_DATA);
		// ---

		// bind extra renderer uniforms ---
		rawrbox::RENDERER->bindRenderUniforms();
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->_s_albedo, texture);
		} else {
			bgfx::setTexture(rawrbox::SAMPLE_MAT_ALBEDO, this->_s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() {
		bgfx::submit(rawrbox::CURRENT_VIEW_ID, this->_program);
	}

	void MaterialBase::upload() {
		rawrbox::RenderUtils::buildShader(clustered_unlit_shaders, this->_program);
	}

} // namespace rawrbox
