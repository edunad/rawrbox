#pragma once

#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {
	struct MaterialTextUniforms {
		//  CAMERA -----
		rawrbox::Matrix4x4 _gWorldViewModel;
		rawrbox::Matrix4x4 _gInvView;
		//  --------

		rawrbox::Vector4f _gBillboard;
	};

	class MaterialText3D : public rawrbox::MaterialUnlit {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		void prepareMaterial() override;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialText3D() = default;
		MaterialText3D(const MaterialText3D&) = delete;
		MaterialText3D(MaterialText3D&&) = delete;
		MaterialText3D& operator=(const MaterialText3D&) = delete;
		MaterialText3D& operator=(MaterialText3D&&) = delete;
		~MaterialText3D() override = default;

		static void init();

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto renderer = rawrbox::RENDERER;
			auto context = renderer->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialTextUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			// Map the buffer and write current world-view-projection matrix

			auto tTransform = rawrbox::TRANSFORM;
			tTransform.transpose();

			auto tWorldView = renderer->camera()->getProjViewMtx();
			tWorldView.transpose();

			auto tInvView = renderer->camera()->getViewMtx();
			tInvView.transpose();
			tInvView.inverse();

			*CBConstants = {
			    // CAMERA -------
			    tTransform * tWorldView,
			    tInvView,
			    // --------------
			    mesh.getData("billboard_mode")};
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			this->prepareMaterial();
			auto context = rawrbox::RENDERER->context();

			if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
				mesh.texture->update(); // Update texture
				this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(mesh.texture->getHandle());
			} else {
				this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(rawrbox::WHITE_TEXTURE->getHandle());
			}
		}
	};

} // namespace rawrbox
