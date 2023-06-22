#pragma once
#include <rawrbox/render/model/material/lit.hpp>

namespace rawrbox {

	class MaterialSkinnedLit : public rawrbox::MaterialLit {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override;

		void setBoneData(const std::vector<rawrbox::Matrix4x4>& data);
		void registerUniforms() override;
		void upload() override;

		static const bgfx::VertexLayout vLayout() {
			return rawrbox::VertexData::vLayout(true, true);
		}
	};
} // namespace rawrbox
