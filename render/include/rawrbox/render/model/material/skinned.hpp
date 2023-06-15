#pragma once

#include <rawrbox/render/model/material/base.hpp>

namespace rawrbox {

	class MaterialSkinnedUnlit : public rawrbox::MaterialBase {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexSkinnedUnlitData;

		MaterialSkinnedUnlit() = default;
		MaterialSkinnedUnlit(MaterialSkinnedUnlit&&) = delete;
		MaterialSkinnedUnlit& operator=(MaterialSkinnedUnlit&&) = delete;
		MaterialSkinnedUnlit(const MaterialSkinnedUnlit&) = delete;
		MaterialSkinnedUnlit& operator=(const MaterialSkinnedUnlit&) = delete;
		~MaterialSkinnedUnlit() override;

		void setBoneData(const std::vector<rawrbox::Matrix4x4>& data);
		void registerUniforms() override;
		void upload() override;
	};

} // namespace rawrbox