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
namespace rawrBox {
	class MaterialLit : public rawrBox::MaterialBase {
	public:
		bgfx::UniformHandle s_texSpecularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_texSpecularShininess = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsSetting = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsData = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrBox::VertexLitData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() {
			RAWRBOX_DESTROY(s_texSpecularColor);

			RAWRBOX_DESTROY(u_texSpecularShininess);
			RAWRBOX_DESTROY(u_lightsSetting);
			RAWRBOX_DESTROY(u_lightsPosition);
			RAWRBOX_DESTROY(u_lightsData);
		}

		void registerUniforms() {
			MaterialBase::registerUniforms();

			// LIT ----
			s_texSpecularColor = bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler);
			u_texSpecularShininess = bgfx::createUniform("u_texSpecularShininess", bgfx::UniformType::Vec4, 1);

			u_lightsSetting = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4, 2);
			u_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrBox::MAX_LIGHTS);
			u_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrBox::MAX_LIGHTS);
			// ---
		}

		void preProcess(const rawrBox::Vector3f& camPos) {
			auto& lightManager = rawrBox::LightManager::getInstance();
			size_t lightCount = lightManager.count();

			std::array lightSettings = {lightManager.fullbright ? 1.F : 0.F, static_cast<float>(lightCount)};
			bgfx::setUniform(u_lightsSetting, lightSettings.data());

			if (lightSettings[0] == 1.F || lightCount <= 0) return; // Fullbright

			std::vector<std::array<float, 16>> lightData(lightCount);
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
		void process(std::shared_ptr<rawrBox::Mesh<T>> mesh) {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, s_texColor, mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, s_texColor, rawrBox::WHITE_TEXTURE->getHandle());
			}

			if (mesh->specularTexture != nullptr && mesh->specularTexture->valid() && !mesh->wireframe) {
				bgfx::setTexture(1, s_texSpecularColor, mesh->specularTexture->getHandle());
			} else {
				bgfx::setTexture(1, s_texSpecularColor, rawrBox::MISSING_SPECULAR_TEXTURE->getHandle());
			}

			std::array shininess = {mesh->specularShininess};
			bgfx::setUniform(u_texSpecularShininess, shininess.data());

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(u_colorOffset, colorOffset.data());
		}

		void upload() {
			buildShader(model_lit_shaders, "model_lit");
		}
	};
} // namespace rawrBox
