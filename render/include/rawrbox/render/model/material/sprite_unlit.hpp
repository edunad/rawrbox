#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/shader_defines.h>
#include <rawrbox/render/static.h>

#include <bgfx/bgfx.h>
#include <fmt/format.h>
#include <generated/shaders/render/all.h>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_sprite_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_sprite_unlit),
    BGFX_EMBEDDED_SHADER(fs_sprite_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {

	class MaterialSpriteUnlit : public rawrBox::MaterialBase {

	public:
		bool fullbright = false;

		MaterialSpriteUnlit() : MaterialBase() {
			this->registerUniform("s_texColor", bgfx::UniformType::Sampler);

			this->registerUniform("u_sprite_pos", bgfx::UniformType::Vec4, 3); // ¯\_(ツ)_/¯ hate it
			this->registerUniform("u_colorOffset", bgfx::UniformType::Vec4, 2);
		};

		void upload() override {
			this->buildShader(model_sprite_shaders, "sprite_unlit");
		}

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, this->getUniform("s_texColor"), mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->getUniform("s_texColor"), rawrBox::MISSING_TEXTURE->getHandle());
			}

			auto pos = rawrBox::Vector3f(mesh->vertexPos[12], mesh->vertexPos[13], mesh->vertexPos[14]);

			this->setUniform("u_sprite_pos", pos);
			this->setUniform("u_colorOffset", mesh->color);
		}
	};
} // namespace rawrBox
