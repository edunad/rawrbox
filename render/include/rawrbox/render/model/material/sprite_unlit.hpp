#pragma once

#include <rawrbox/math/quaternion.hpp>
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
		bgfx::UniformHandle s_texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_sprite_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		MaterialSpriteUnlit() : MaterialBase(){};
		MaterialSpriteUnlit(MaterialSpriteUnlit&&) = delete;
		MaterialSpriteUnlit& operator=(MaterialSpriteUnlit&&) = delete;
		MaterialSpriteUnlit(const MaterialSpriteUnlit&) = delete;
		MaterialSpriteUnlit& operator=(const MaterialSpriteUnlit&) = delete;
		~MaterialSpriteUnlit() override {
			MaterialBase::~MaterialBase();

			RAWRBOX_DESTROY(this->s_texColor);
			RAWRBOX_DESTROY(this->u_sprite_pos);
			RAWRBOX_DESTROY(this->u_colorOffset);
		}

		void registerUniforms() override {
			MaterialBase::registerUniforms();

			this->s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

			this->u_sprite_pos = bgfx::createUniform("u_sprite_pos", bgfx::UniformType::Vec4, 3); // ¯\_(ツ)_/¯ hate it
			this->u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);
		}

		void upload() override {
			this->buildShader(model_sprite_shaders, "sprite_unlit");
		}

		void process(std::shared_ptr<rawrBox::Mesh> mesh) override {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, this->s_texColor, mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, this->s_texColor, rawrBox::MISSING_TEXTURE->getHandle());
			}

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(this->u_colorOffset, colorOffset.data());

			std::array offset = {mesh->vertexPos[12], mesh->vertexPos[13], mesh->vertexPos[14]};
			bgfx::setUniform(this->u_sprite_pos, offset.data());
		}
	};
} // namespace rawrBox
