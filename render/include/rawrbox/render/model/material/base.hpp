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

		bgfx::UniformHandle s_albedo = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_normal = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_colorOffset = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_mesh_pos = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_data = BGFX_INVALID_HANDLE;

		// LIT DATA ---
		bgfx::UniformHandle s_specular = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_emission = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_opacity = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_specularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_emissionColor = BGFX_INVALID_HANDLE;

		/*bgfx::UniformHandle u_texMatData = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_lightsSetting = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsData = BGFX_INVALID_HANDLE;*/
		//------

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
	concept supportsBones = requires(T t, const std::vector<rawrbox::Matrix4x4>& data) {
		{ t.setBoneData(data) };
	};
	// ---

} // namespace rawrbox
