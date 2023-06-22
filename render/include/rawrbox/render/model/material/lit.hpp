#pragma once

#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {
	class MaterialLit : public rawrbox::MaterialBase {
	public:
		bgfx::UniformHandle s_texSpecularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_texEmissionColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle s_texOpacityColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_specularColor = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_emissionColor = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_texMatData = BGFX_INVALID_HANDLE;

		bgfx::UniformHandle u_lightsSetting = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsPosition = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle u_lightsData = BGFX_INVALID_HANDLE;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override;

		void registerUniforms() override;
		void preProcess() override;

		void process(const rawrbox::Mesh& mesh) override;
		void upload() override;

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout(true, false);
		}
	};
} // namespace rawrbox
