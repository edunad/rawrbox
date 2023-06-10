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
		virtual void buildShader(const bgfx::EmbeddedShader shaders[]);
		// NOLINTEND(hicpp-avoid-c-arrays)

		virtual void registerUniforms();
		virtual void preProcess(const rawrbox::Vector3f& camPos);

		template <typename T>
		void process(const rawrbox::Mesh<T>& mesh) {
			if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.wireframe) {
				bgfx::setTexture(0, s_texColor, mesh.texture->getHandle());
			} else {
				bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
			}

			bgfx::setUniform(u_colorOffset, mesh.color.data().data());

			std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
			bgfx::setUniform(u_mesh_pos, offset.data());

			// Pass "special" data ---
			std::array<float, 4> data = {0.F, 0.F, 0.F, 0.F};
			if (mesh.hasData("billboard_mode")) {
				data[0] = mesh.getData("billboard_mode").x;
			}

			if (mesh.hasData("vertex_snap")) {
				data[1] = mesh.getData("vertex_snap").x;
			}

			bgfx::setUniform(u_data, data.data());
			// ---
		}

		virtual void process(const bgfx::TextureHandle& texture);
		virtual void postProcess();
		virtual void upload();
	};

} // namespace rawrbox
