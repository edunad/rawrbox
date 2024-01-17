#pragma once

#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/materials/skinned.hpp>

namespace rawrbox {
	class MaterialSkinnedLit : public rawrbox::MaterialLit {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexNormBoneData;

		MaterialSkinnedLit() = default;
		MaterialSkinnedLit(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit& operator=(MaterialSkinnedLit&&) = delete;
		MaterialSkinnedLit(const MaterialSkinnedLit&) = delete;
		MaterialSkinnedLit& operator=(const MaterialSkinnedLit&) = delete;
		~MaterialSkinnedLit() override = default;

		void init() override;
	};

} // namespace rawrbox
