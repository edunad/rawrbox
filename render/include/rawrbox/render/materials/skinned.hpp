#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	struct MaterialSkinnedUniforms : public rawrbox::MaterialBaseUniforms {
		std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> g_bones;
	};

	class MaterialSkinned : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		void bindUniforms(const rawrbox::Mesh<rawrbox::VertexData>& mesh) override;
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
		[[nodiscard]] const uint32_t vLayoutSize() override;
	};

} // namespace rawrbox
