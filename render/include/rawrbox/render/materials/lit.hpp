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

		void setupUniforms() override;

	public:
		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override;

		void process(const rawrbox::Mesh& mesh) override;
		void postProcess() override;
		void upload() override;

		[[nodiscard]] uint32_t supports() const override;
		[[nodiscard]] const bgfx::VertexLayout vLayout() const override;
	};
} // namespace rawrbox
