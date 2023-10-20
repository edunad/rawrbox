#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	struct MaterialTextUniforms {
		//  CAMERA -----
		rawrbox::Matrix4x4 _gWorldViewModel;
		rawrbox::Matrix4x4 _gInvView;
		//  --------

		rawrbox::Vector4f _gBillboard;
	};

	class MaterialText3D : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		static Diligent::IPipelineState* _base;
		static Diligent::IPipelineState* _wireframe;

		static Diligent::IShaderResourceBinding* _bind;

	protected:
		void bindUniforms(const rawrbox::Mesh& mesh) override;
		void bindPipeline(const rawrbox::Mesh& mesh) override;

	public:
		MaterialText3D() = default;
		MaterialText3D(const MaterialText3D&) = delete;
		MaterialText3D(MaterialText3D&&) = delete;
		MaterialText3D& operator=(const MaterialText3D&) = delete;
		MaterialText3D& operator=(MaterialText3D&&) = delete;
		~MaterialText3D() override = default;

		static void init();
		void bind(const rawrbox::Mesh& mesh) override;

		[[nodiscard]] uint32_t supports() const override;
	};

} // namespace rawrbox
