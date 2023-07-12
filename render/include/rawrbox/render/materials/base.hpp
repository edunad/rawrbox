#pragma once

#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {

	class MaterialBase {
	protected:
		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _s_albedo = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _s_displacement = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _u_camPos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _u_mesh_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _u_data = BGFX_INVALID_HANDLE;

	public:
		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase();

		virtual void registerUniforms();

		virtual void process(const rawrbox::Mesh& mesh);
		virtual void process(const bgfx::TextureHandle& texture);

		virtual void postProcess();
		virtual void upload();

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout();
		}
	};

	// UTILS ---
	template <typename T>
	concept supportsNormals = requires(T t) { t.u_texMatData; };

	template <typename T>
	concept supportsBones = requires(T t, const std::vector<rawrbox::Matrix4x4>& data) {
		{ t.setBoneData(data) };
	};
	// ---

} // namespace rawrbox
