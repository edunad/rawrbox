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
		using MaterialBase::MaterialBase;

		void registerUniforms() override {
			MaterialBase::registerUniforms();

			this->registerUniform("s_texColor", bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler));
			this->registerUniform("u_colorOffset", bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4));
		}

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, this->getUniform("s_texColor"), mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->getUniform("s_texColor"), rawrBox::MISSING_TEXTURE->getHandle());
			}

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(this->getUniform("u_colorOffset"), colorOffset.data());
		}

		void upload() override {
			this->buildShader(model_unlit_shaders, "model_unlit");
		}
	};
} // namespace rawrBox
