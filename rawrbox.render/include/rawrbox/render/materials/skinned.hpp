#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	class MaterialSkinned : public rawrbox::MaterialUnlit {
		static bool _built;

	public:
#ifdef _DEBUG
		static bool DEBUG_MODE;
#endif

		using vertexBufferType = rawrbox::VertexBoneData;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

		void init() override;

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		bool bindVertexSkinnedUniforms(const rawrbox::Mesh<T>& mesh) {
			const rawrbox::BindlessVertexSkinnedBuffer buff = this->bindBaseVertexSkinnedUniforms<T>(mesh);
			return rawrbox::UniformCache::map(rawrbox::BindlessManager::signatureBufferVertexSkinned, buff);
		}
	};

} // namespace rawrbox