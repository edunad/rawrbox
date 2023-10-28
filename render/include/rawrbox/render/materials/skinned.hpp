#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	struct MaterialSkinnedUniforms : public rawrbox::MaterialBaseUniforms {
		std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> g_bones;
	};

	class MaterialSkinned : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		std::vector<rawrbox::VertexBoneData> _temp = {};

	protected:
		void bindUniforms(const rawrbox::Mesh& mesh) override;
		void prepareMaterial() override;

	public:
		MaterialSkinned() = default;
		MaterialSkinned(MaterialSkinned&&) = delete;
		MaterialSkinned& operator=(MaterialSkinned&&) = delete;
		MaterialSkinned(const MaterialSkinned&) = delete;
		MaterialSkinned& operator=(const MaterialSkinned&) = delete;
		~MaterialSkinned() override = default;

		static void init();

		void* convert(const std::vector<rawrbox::ModelVertexData>& v) override;

		[[nodiscard]] uint32_t supports() const override;
		[[nodiscard]] const uint32_t vLayoutSize() override;
	};

} // namespace rawrbox
