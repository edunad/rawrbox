#pragma once

#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/render/model/defs.hpp>
#include <rawrbox/render/model/mesh.hpp>

#include <bgfx/bgfx.h>
#include <fmt/format.h>

namespace rawrbox {

	class MaterialLit : public rawrbox::MaterialBase {
	protected:
		bgfx::ProgramHandle _debug_z_program = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _debug_program = BGFX_INVALID_HANDLE;

		// LIT DATA ---
		bgfx::UniformHandle _s_normal = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _s_specular = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle _s_emission = BGFX_INVALID_HANDLE;
		//------
	public:
		bgfx::UniformHandle u_texMatData = BGFX_INVALID_HANDLE;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override;

		void registerUniforms() override;
		void process(const rawrbox::Mesh& mesh) override;

		void postProcess() override;
		void upload() override;

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout(true);
		}
	};
} // namespace rawrbox
