#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialDecal : public rawrbox::MaterialBase {
	protected:
		bgfx::UniformHandle _s_depth = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _u_invModel = BGFX_INVALID_HANDLE;

	public:
		MaterialDecal() = default;
		MaterialDecal(MaterialDecal&&) = delete;
		MaterialDecal& operator=(MaterialDecal&&) = delete;
		MaterialDecal(const MaterialDecal&) = delete;
		MaterialDecal& operator=(const MaterialDecal&) = delete;
		~MaterialDecal() override;

		void registerUniforms() override;
		void process(const rawrbox::Mesh& mesh) override;

		void upload() override;
	};

} // namespace rawrbox
