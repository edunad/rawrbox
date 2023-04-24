#pragma once

#include <rawrbox/render/model/light/manager.h>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bgfx/bgfx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {

	class MaterialUnlit : public rawrBox::MaterialBase {

	public:
		bgfx::UniformHandle s_texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		MaterialUnlit() : MaterialBase(){};
		MaterialUnlit(MaterialUnlit&&) = delete;
		MaterialUnlit& operator=(MaterialUnlit&&) = delete;
		MaterialUnlit(const MaterialUnlit&) = delete;
		MaterialUnlit& operator=(const MaterialUnlit&) = delete;
		~MaterialUnlit() override {
			MaterialBase::~MaterialBase();

			RAWRBOX_DESTROY(this->s_texColor);
			RAWRBOX_DESTROY(this->u_colorOffset);
		}

		void registerUniforms() override {
			MaterialBase::registerUniforms();

			this->s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);
			this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		}

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, this->s_texColor, mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->s_texColor, rawrBox::MISSING_TEXTURE->getHandle());
			}

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(this->u_colorOffset, colorOffset.data());
		}

		void upload() override {
			this->buildShader(model_unlit_shaders, "model_unlit");
		}
	};
} // namespace rawrBox
