#pragma once

#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bgfx/bgfx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_lit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_lit),
    BGFX_EMBEDDED_SHADER(fs_model_lit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)
namespace rawrBox {

	class MaterialLit : public rawrBox::MaterialBase {

	public:
		bgfx::UniformHandle s_texColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle s_texSpecularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_texShininess = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_lightsSetting = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsData = BGFX_INVALID_HANDLE;

		MaterialLit() : MaterialBase(){};
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override {
			MaterialBase::~MaterialBase();

			RAWRBOX_DESTROY(this->s_texColor);
			RAWRBOX_DESTROY(this->s_texSpecularColor);
			RAWRBOX_DESTROY(this->u_texShininess);

			RAWRBOX_DESTROY(this->u_colorOffset);
			RAWRBOX_DESTROY(this->u_lightsSetting);
			RAWRBOX_DESTROY(this->u_lightsPosition);
			RAWRBOX_DESTROY(this->u_lightsData);
		}

		void registerUniforms() override {
			MaterialBase::registerUniforms();

			this->s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
			this->s_texSpecularColor = bgfx::createUniform("s_texSpecularColor", bgfx::UniformType::Sampler);

			this->u_texShininess = bgfx::createUniform("u_texSpecularShininess", bgfx::UniformType::Vec4, 1);
			this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);

			this->u_lightsSetting = bgfx::createUniform("u_lightsSetting", bgfx::UniformType::Vec4, 2);
			this->u_lightsPosition = bgfx::createUniform("u_lightsPosition", bgfx::UniformType::Vec4, rawrBox::LightManager::getInstance().maxLights);
			this->u_lightsData = bgfx::createUniform("u_lightsData", bgfx::UniformType::Mat4, rawrBox::LightManager::getInstance().maxLights);
		}

		void preProcess(const rawrBox::Vector3f& camPos) override {
			auto& lightManager = rawrBox::LightManager::getInstance();
			size_t lightCount = lightManager.count();

			std::array lightSettings = {lightManager.fullbright ? 1.f : 0.f, static_cast<float>(lightCount)};
			bgfx::setUniform(u_lightsSetting, lightSettings.data());

			if (lightSettings[0] == 1.f || lightCount <= 0) return; // Fullbright

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

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, this->s_texColor, mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->s_texColor, rawrBox::MISSING_TEXTURE->getHandle());
			}

			if (mesh->specularTexture != nullptr && mesh->specularTexture->valid() && !mesh->wireframe) {
				bgfx::setTexture(1, this->s_texSpecularColor, mesh->specularTexture->getHandle());
			} else {
				bgfx::setTexture(1, this->s_texSpecularColor, rawrBox::MISSING_SPECULAR_TEXTURE->getHandle());
			}

			std::array shininess = {mesh->specularShininess};
			bgfx::setUniform(u_texShininess, shininess.data());

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(u_colorOffset, colorOffset.data());
		}

		void upload() override {
			this->buildShader(model_lit_shaders, "model_lit");
		}
	};
} // namespace rawrBox
