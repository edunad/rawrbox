#pragma once
#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	class MaterialSkinned : public rawrbox::MaterialBase {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override;

		void setBoneData(const std::vector<rawrbox::Matrix4x4>& data);
		void registerUniforms() override;
		void upload() override;

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout(true);
		}
	};
} // namespace rawrbox
