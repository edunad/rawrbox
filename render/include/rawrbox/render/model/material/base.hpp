#pragma once

#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/shader_defines.hpp>
#include <rawrbox/render/static.hpp>

#include <generated/shaders/render/all.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

#include <stdexcept>

// NOLINTBEGIN(*)
static const bgfx::EmbeddedShader model_unlit_shaders[] = {
    BGFX_EMBEDDED_SHADER(vs_model_unlit),
    BGFX_EMBEDDED_SHADER(fs_model_unlit),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

namespace rawrBox {

	class MaterialBase {
	public:
		bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle s_texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_viewPos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_mesh_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_billboard = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrBox::VertexData;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		~MaterialBase() {
			RAWRBOX_DESTROY(program);

			RAWRBOX_DESTROY(s_texColor);
			RAWRBOX_DESTROY(u_viewPos);
			RAWRBOX_DESTROY(u_colorOffset);

			RAWRBOX_DESTROY(u_mesh_pos);
			RAWRBOX_DESTROY(u_billboard);
		}

		// NOLINTBEGIN(hicpp-avoid-c-arrays)
		void buildShader(const bgfx::EmbeddedShader shaders[], const std::string& name) {
			bgfx::RendererType::Enum type = bgfx::getRendererType();
			bgfx::ShaderHandle vsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("vs_{}", name).c_str());
			bgfx::ShaderHandle fsh = bgfx::createEmbeddedShader(shaders, type, fmt::format("fs_{}", name).c_str());

			program = bgfx::createProgram(vsh, fsh, true);
			if (!bgfx::isValid(program)) throw std::runtime_error("[RawrBox-MaterialBase] Failed to create shader");
		}
		// NOLINTEND(hicpp-avoid-c-arrays)

		void registerUniforms() {
			s_texColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Sampler);

			u_viewPos = bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4, 3);
			u_colorOffset = bgfx::createUniform("u_colorOffset", bgfx::UniformType::Vec4);

			u_mesh_pos = bgfx::createUniform("u_mesh_pos", bgfx::UniformType::Vec4, 3);
			u_billboard = bgfx::createUniform("u_billboard", bgfx::UniformType::Vec4, 1);
		}

		void preProcess(const rawrBox::Vector3f& camPos) {
			std::array pos = {camPos.x, camPos.y, camPos.z};
			bgfx::setUniform(u_viewPos, pos.data());
		}

		template <typename T>
		void process(std::shared_ptr<rawrBox::Mesh<T>> mesh) {
			if (mesh->texture != nullptr && mesh->texture->valid() && !mesh->wireframe) {
				bgfx::setTexture(0, s_texColor, mesh->texture->getHandle());
			} else {
				bgfx::setTexture(0, s_texColor, rawrBox::WHITE_TEXTURE->getHandle());
			}

			std::array colorOffset = {mesh->color.r, mesh->color.b, mesh->color.g, mesh->color.a};
			bgfx::setUniform(u_colorOffset, colorOffset.data());

			std::array offset = {mesh->vertexPos[12], mesh->vertexPos[13], mesh->vertexPos[14]};
			bgfx::setUniform(u_mesh_pos, offset.data());

			std::array<float, 2> billboard = {0.F, 0.F};
			if (mesh->hasData("billboard_mode")) {
				billboard[0] = mesh->getData("billboard_mode").x;
			}

			bgfx::setUniform(u_billboard, billboard.data());
		}

		void postProcess() { bgfx::submit(rawrBox::CURRENT_VIEW_ID, program); }
		void upload() {
			MaterialBase::buildShader(model_unlit_shaders, "model_unlit");
		}
	};

} // namespace rawrBox
