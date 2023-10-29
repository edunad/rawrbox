#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	struct MaterialSkinnedUniforms : public rawrbox::MaterialBaseUniforms {
		std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> g_bones;
	};

	class MaterialSkinned : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		void prepareMaterial() override;

	public:
		using vertexBufferType = rawrbox::VertexBoneData;

		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

		static void init();

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialSkinnedUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialSkinnedUniforms>(mesh, CBConstants);
			// ------------

			(*CBConstants).g_bones = mesh.boneTransforms;
		}
	};

} // namespace rawrbox
