#pragma once

#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/shader_defines.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {

	class MaterialBase {
	public:
		bgfx::ProgramHandle program = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle s_texColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_mesh_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_data = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexData;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase();

		// NOLINTBEGIN(hicpp-avoid-c-arrays)
		virtual void buildShader(const bgfx::EmbeddedShader shaders[], const std::string& name);
		// NOLINTEND(hicpp-avoid-c-arrays)

		virtual void registerUniforms();
		virtual void preProcess(const rawrbox::Vector3f& camPos);

		template <typename T>
		void process(const rawrbox::Mesh<T>& mesh) {
			/*if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.wireframe) {
				bgfx::setTexture(0, s_texColor, mesh.texture->getHandle());
			} else {
				bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
			}

			bgfx::setUniform(u_colorOffset, mesh.color.data().data());

			std::array<float, 1> billboard = {0.F};
			if (mesh.hasData("billboard_mode")) {
				billboard[0] = mesh.getData("billboard_mode").x;

				std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
				bgfx::setUniform(u_mesh_pos, offset.data());
			}

			bgfx::setUniform(u_data, billboard.data());*/
			bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
		}

		virtual void process(const bgfx::TextureHandle& texture);
		virtual void postProcess();
		virtual void upload();
	};

} // namespace rawrbox
