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
		bgfx::UniformHandle s_texBumpColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_mesh_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_data = BGFX_INVALID_HANDLE;

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
		virtual void preProcess();

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout();
		}

		virtual void process(const rawrbox::Mesh& mesh) {
			if (mesh.texture != nullptr && mesh.texture->valid() && !mesh.lineMode && !mesh.wireframe) {
				bgfx::setTexture(0, s_texColor, mesh.texture->getHandle());
			} else {
				bgfx::setTexture(0, s_texColor, rawrbox::WHITE_TEXTURE->getHandle());
			}

			if (mesh.bumpTexture != nullptr && mesh.bumpTexture->valid()) {
				bgfx::setTexture(1, s_texBumpColor, mesh.bumpTexture->getHandle());
			} else {
				bgfx::setTexture(1, s_texBumpColor, rawrbox::BLACK_TEXTURE->getHandle());
			}

			// Color override
			bgfx::setUniform(u_colorOffset, mesh.color.data().data());
			// -------

			// Mesh pos
			std::array offset = {mesh.vertexPos[12], mesh.vertexPos[13], mesh.vertexPos[14]};
			bgfx::setUniform(u_mesh_pos, offset.data());
			// -------

			// Pass "special" data ---
			std::array<std::array<float, 4>, 4> data = {std::array<float, 4>{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};
			if (mesh.hasData("billboard_mode")) {
				data[0] = mesh.getData("billboard_mode").data();
			}

			if (mesh.hasData("vertex_snap")) {
				data[1] = mesh.getData("vertex_snap").data();
			}

			if (mesh.hasData("displacement_strength")) {
				data[2] = mesh.getData("displacement_strength").data();
			}

			bgfx::setUniform(u_data, data.front().data(), 4);
			// ---
		}

		virtual void process(const bgfx::TextureHandle& texture);
		virtual void postProcess();
		virtual void upload();
	};

	// UTILS ---
	template <typename T>
	concept supportsBones = requires(T t, const std::vector<rawrbox::Matrix4x4>& data) {
		{ t.setBoneData(data) };
	};
	// ---

} // namespace rawrbox
