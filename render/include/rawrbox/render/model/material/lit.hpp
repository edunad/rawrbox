#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_lit),
    BGFX_EMBEDDED_SHADER(fs_model_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)
namespace rawrbox {
	class MaterialLit : public rawrbox::MaterialBase {
	public:
		bgfx::UniformHandle s_texSpecularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_texEmissionColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_specularColorOffset = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_emissionColorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_texMatData = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_lightsSetting = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsData = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexLitData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override {
			RAWRBOX_DESTROY(s_texSpecularColor);
			RAWRBOX_DESTROY(s_texEmissionColor);

			RAWRBOX_DESTROY(u_specularColorOffset);
			RAWRBOX_DESTROY(u_emissionColorOffset);

			RAWRBOX_DESTROY(u_texMatData);

			RAWRBOX_DESTROY(u_lightsSetting);
			RAWRBOX_DESTROY(u_lightsPosition);
			RAWRBOX_DESTROY(u_lightsData);
		}

		void registerUniforms() {
			MaterialBase::registerUniforms();

			// LIT ----
			s_texSpecularColor = bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler);
			u_texMatData = bgfx::createUniform("u_texMatData", bgfx::UniformType::Vec4, 2);

			u_specularColorOffset = bgfx::createUniform("u_colorSpecularOffset", bgfx::UniformType::Vec4);
			u_emissionColorOffset = bgfx::createUniform("u_emissionColorOffset", bgfx::UniformType::Vec4);

			s_texEmissionColor = bgfx::createUniform("s_texEmissionColor", bgfx::UniformType::Sampler);

			u_lightsSetting = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4, 2);
			u_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrbox::MAX_LIGHTS);
			u_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrbox::MAX_LIGHTS);
			// ---
		}

		void preProcess(const rawrbox::Vector3f& camPos) {
			rawrbox::MaterialBase::preProcess(camPos);

			auto& lightManager = rawrbox::LightManager::get();
			size_t lightCount = lightManager.count();

			std::array lightSettings = {lightManager.fullbright ? 1.F : 0.F, static_cast<float>(lightCount)};
			bgfx::setUniform(u_lightsSetting, lightSettings.data());

			if (lightSettings[0] == 1.F || lightCount <= 0) return; // Fullbright

			std::vector<rawrbox::Matrix4x4> lightData(lightCount);
			std::vector<std::array<float, 4>> lightPos(lightCount);

			for (size_t i = 0; i < lightCount; i++) {
				auto light = lightManager.getLight(i);

				lightPos[i] = light->getPosMatrix();
				lightData[i] = light->getDataMatrix();
			}

			if (lightPos.size() != lightData.size()) throw std::runtime_error("[RawrBox-MODEL] LightPos and LightData do not match!");

			bgfx::setUniform(u_lightsPosition, lightPos.front().data(), static_cast<uint16_t>(lightCount));
			bgfx::setUniform(u_lightsData, lightData.front().data(), static_cast<uint16_t>(lightCount));
		}

		template <typename T>
		void process(std::shared_ptr<rawrbox::Mesh<T>> mesh) {
			rawrbox::MaterialBase::process(mesh);

			if (mesh->specularTexture != nullptr && mesh->specularTexture->valid() && !mesh->wireframe) {
				bgfx::setTexture(1, s_texSpecularColor, mesh->specularTexture->getHandle());
			} else {
				bgfx::setTexture(1, s_texSpecularColor, rawrbox::MISSING_SPECULAR_TEXTURE->getHandle());
			}

			if (mesh->emissionTexture != nullptr && mesh->emissionTexture->valid() && !mesh->wireframe) {
				bgfx::setTexture(2, s_texEmissionColor, mesh->emissionTexture->getHandle());
			} else {
				bgfx::setTexture(2, s_texEmissionColor, rawrbox::MISSING_SPECULAR_TEXTURE->getHandle());
			}

			std::array<float, 2> matData = {mesh->specularShininess, mesh->emissionIntensity};
			bgfx::setUniform(u_texMatData, matData.data());

			bgfx::setUniform(u_specularColorOffset, mesh->specularColor.data().data());
			bgfx::setUniform(u_emissionColorOffset, mesh->emissionColor.data().data());
		}

		void upload() {
			buildShader(model_lit_shaders, "model_lit");
		}
	};
} // namespace rawrbox
