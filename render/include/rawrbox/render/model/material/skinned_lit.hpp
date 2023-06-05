#pragma once
#include <rawrbox/render/model/material/lit.hpp>

namespace rawrbox {

	class MaterialSkinnedLit : public rawrbox::MaterialLit {
	public:
		bgfx::UniformHandle u_bones = BGFX_INVALID_HANDLE;

		using vertexBufferType = rawrbox::VertexSkinnedLitData;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override;

		void registerUniforms() override;
		void upload() override;
	};
} // namespace rawrbox
