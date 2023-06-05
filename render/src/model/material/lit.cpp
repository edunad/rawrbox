
#include <rawrbox/render/model/light/base.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/material/lit.hpp>

#include <generated/shaders/render/all.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader model_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_lit),
    BGFX_EMBEDDED_SHADER(fs_model_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrbox {
	MaterialLit::~MaterialLit() {
		RAWRBOX_DESTROY(s_texSpecularColor);
		RAWRBOX_DESTROY(s_texEmissionColor);
		RAWRBOX_DESTROY(s_texOpacityColor);

		RAWRBOX_DESTROY(u_specularColor);
		RAWRBOX_DESTROY(u_emissionColor);

		RAWRBOX_DESTROY(u_texMatData);
		RAWRBOX_DESTROY(u_cameraPos);

		RAWRBOX_DESTROY(u_lightsSetting);
		RAWRBOX_DESTROY(u_lightsPosition);
		RAWRBOX_DESTROY(u_lightsData);
	}

	void MaterialLit::registerUniforms() {
		MaterialBase::registerUniforms();

		// LIT ----
		u_texMatData = bgfx::createUniform("u_texMatData", bgfx::UniformType::Vec4);
		u_cameraPos = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4, 3);

		s_texSpecularColor = bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler);
		s_texEmissionColor = bgfx::createUniform("s_texEmissionColor", bgfx::UniformType::Sampler);
		s_texOpacityColor = bgfx::createUniform("s_texOpacityColor", bgfx::UniformType::Sampler);

		u_specularColor = bgfx::createUniform("u_specularColor", bgfx::UniformType::Vec4);
		u_emissionColor = bgfx::createUniform("u_emissionColor", bgfx::UniformType::Vec4);

		u_lightsSetting = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4);
		u_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrbox::MAX_LIGHTS);
		u_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrbox::MAX_LIGHTS);
		// ---
	}

	void MaterialLit::preProcess(const rawrbox::Vector3f& camPos) {
		rawrbox::MaterialBase::preProcess(camPos);

		size_t lightCount = rawrbox::LIGHTS::count();

		std::array lightSettings = {rawrbox::LIGHTS::fullbright ? 1.F : 0.F, static_cast<float>(lightCount)};
		bgfx::setUniform(u_lightsSetting, lightSettings.data());

		if (lightSettings[0] == 1.F || lightCount <= 0) return; // Fullbright

		std::vector<rawrbox::Matrix4x4> lightData(lightCount);
		std::vector<std::array<float, 4>> lightPos(lightCount);

		for (size_t i = 0; i < lightCount; i++) {
			auto light = rawrbox::LIGHTS::getLight(i);

			lightPos[i] = light->getPosMatrix();
			lightData[i] = light->getDataMatrix();
		}

		if (lightPos.size() != lightData.size()) throw std::runtime_error("[RawrBox-MODEL] LightPos and LightData do not match!");

		bgfx::setUniform(u_lightsPosition, lightPos.front().data(), static_cast<uint16_t>(lightCount));
		bgfx::setUniform(u_lightsData, lightData.front().data(), static_cast<uint16_t>(lightCount));

		// Camera position for reflection
		std::array pos = {camPos.x, camPos.y, camPos.z};
		bgfx::setUniform(u_cameraPos, pos.data());
	}

	void MaterialLit::upload() {
		this->buildShader(model_lit_shaders, "model_lit");
	}
} // namespace rawrbox
