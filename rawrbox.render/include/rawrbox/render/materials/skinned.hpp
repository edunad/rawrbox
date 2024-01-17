#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	class MaterialSkinned : public rawrbox::MaterialUnlit {
		static bool _built;

	public:
		using vertexBufferType = rawrbox::VertexBoneData;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

		void init() override;
	};

} // namespace rawrbox
