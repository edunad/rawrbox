
#include <rawrbox/render/model/material/base.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model),
    BGFX_EMBEDDED_SHADER(fs_model),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {

	MaterialBase::~MaterialBase() {
		RAWRBOX_DESTROY(this->program);

		RAWRBOX_DESTROY(this->s_albedo);
		RAWRBOX_DESTROY(this->s_normal);

		RAWRBOX_DESTROY(this->u_colorOffset);

		RAWRBOX_DESTROY(this->u_mesh_pos);
		RAWRBOX_DESTROY(this->u_data);

		// LIT ----
		RAWRBOX_DESTROY(this->s_specular);
		RAWRBOX_DESTROY(this->s_emission);
		RAWRBOX_DESTROY(this->s_opacity);

		RAWRBOX_DESTROY(this->u_specularColor);
		RAWRBOX_DESTROY(this->u_emissionColor);

		/*RAWRBOX_DESTROY(this->u_texMatData);

		RAWRBOX_DESTROY(this->u_lightsSetting);
		RAWRBOX_DESTROY(this->u_lightsPosition);
		RAWRBOX_DESTROY(this->u_lightsData);*/
		// ------
	}

	// NOLINTBEGIN(hicpp-avoid-c-arrays)
	void MaterialBase::buildShader(const bgfx::EmbeddedShader shaders[]) {
		bgfx::RendererType::Enum type = bgfx::getRendererType();
		bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, shaders[0].name);
		bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, shaders[1].name);

		program = bgfx::createProgram(vsh, fsh, true);
		if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");
	}
	// NOLINTEND(hicpp-avoid-c-arrays)

	void MaterialBase::registerUniforms() {
		this->s_albedo = bgfx::createUniform("s_albedo", bgfx::UniformType::Sampler);
		this->s_normal = bgfx::createUniform("s_normal", bgfx::UniformType::Sampler);
		this->s_specular = bgfx::createUniform("s_specular", bgfx::UniformType::Sampler);
		this->s_emission = bgfx::createUniform("s_emission", bgfx::UniformType::Sampler);
		this->s_opacity = bgfx::createUniform("s_opacity", bgfx::UniformType::Sampler);

		this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		this->u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4, 3);
		this->u_data = bgfx::createUniform("u_data", bgfx::UniformType::Vec4, 4);

		// LIT ----
		// this->u_texMatData = bgfx::createUniform("u_texMatData", bgfx::UniformType::Vec4);

		this->u_specularColor = bgfx::createUniform("u_specularColor", bgfx::UniformType::Vec4);
		this->u_emissionColor = bgfx::createUniform("u_emissionColor", bgfx::UniformType::Vec4);

		// this->u_lightsSetting = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4);
		// this->u_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrbox::MAX_LIGHTS);
		// this->u_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrbox::MAX_LIGHTS);
		//  ---
	}

	void MaterialBase::preProcess() {
		/*size_t lightCount = rawrbox::LIGHTS::count();

		std::array lightSettings = {rawrbox::LIGHTS::fullbright ? 1.F : 0.F, static_cast<float>(lightCount)};
		bgfx::setUniform(u_lightsSetting, lightSettings.data());

		if (lightSettings[0] == 1.F || lightCount <= 0) return; // Fullbright

		std::vector<rawrbox::Matrix4x4> lightData(lightCount);
		std::vector<std::array<float, 4>> lightPos(lightCount);

		for (size_t i = 0; i < lightCount; i++) {
			auto& light = rawrbox::LIGHTS::getLight(i);

			lightPos[i] = light.getPosMatrix();
			lightData[i] = light.getDataMatrix();
		}

		if (lightPos.size() != lightData.size()) throw std::runtime_error("[RawrBox-MODEL] LightPos and LightData do not match!");

		bgfx::setUniform(this->u_lightsPosition, lightPos.front().data(), static_cast<uint16_t>(lightCount));
		bgfx::setUniform(this->u_lightsData, lightData.front().data(), static_cast<uint16_t>(lightCount));*/
	}

	void MaterialBase::process(const rawrbox::Mesh& mesh) {
		if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(0, s_albedo, mesh.texture->getHandle());
		} else {
			bgfx::setTexture(0, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}

		if (mesh.bumpTexture != nullptr && mesh.bumpTexture->valid()) {
			bgfx::setTexture(1, s_normal, mesh.bumpTexture->getHandle());
		} else {
			bgfx::setTexture(1, s_normal, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.specularTexture != nullptr && mesh.specularTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(2, s_specular, mesh.specularTexture->getHandle());
		} else {
			bgfx::setTexture(2, s_specular, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.emissionTexture != nullptr && mesh.emissionTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(3, s_emission, mesh.emissionTexture->getHandle());
		} else {
			bgfx::setTexture(3, s_emission, rawrbox::BLACK_TEXTURE->getHandle());
		}

		if (mesh.opacityTexture != nullptr && mesh.opacityTexture->valid() && !mesh.lineMode && !mesh.wireframe) {
			bgfx::setTexture(4, s_opacity, mesh.opacityTexture->getHandle());
		} else {
			bgfx::setTexture(4, s_opacity, rawrbox::WHITE_TEXTURE->getHandle());
		}

		// Extra material data --
		bgfx::setUniform(u_specularColor, mesh.specularColor.data().data());
		bgfx::setUniform(u_emissionColor, mesh.emissionColor.data().data());

		// std::array<float, 2> matData = {mesh.specularShininess, mesh.emissionIntensity};
		// bgfx::setUniform(u_texMatData, matData.data());
		//  ----

		// Color override
		bgfx::setUniform(u_colorOffset, mesh.color.data().data());
		// -------

		// Mesh pos
		std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
		bgfx::setUniform(u_mesh_pos, offset.data());
		// -------

		// Pass "special" data ---
		std::array<std::array<float, 4>, 4> data = {std::array<float, 4>{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
		if (mesh.hasData("billboard_mode")) {
			data[0] = mesh.getData("billboard_mode").data();
		}

		if (mesh.hasData("vertex_snap")) {
			data[1] = mesh.getData("vertex_snap").data();
		}

		if (mesh.hasData("displacement_strength")) {
			data[2] = mesh.getData("displacement_strength").data();
		}

		bgfx::setUniform(u_data, data.front().data(), 4);
		// ---
	}

	void MaterialBase::process(const bgfx::TextureHandle& texture) {
		if (bgfx::isValid(texture)) {
			bgfx::setTexture(0, s_albedo, texture);
		} else {
			bgfx::setTexture(0, s_albedo, rawrbox::WHITE_TEXTURE->getHandle());
		}
	}

	void MaterialBase::postProcess() { bgfx::submit(rawrbox::CURRENT_VIEW_ID, program); }
	void MaterialBase::upload() {
		this->buildShader(model_shaders);
	}

} // namespace rawrbox
