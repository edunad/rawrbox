#pragma once

#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {

	struct MaterialUnlitUniforms : public rawrbox::MaterialBaseUniforms {
		// Model ----
		rawrbox::Colorf _gColorOverride;
		rawrbox::Vector4f _gTextureFlags;

		std::array<rawrbox::Vector4f, 4> _gData; // Other mesh data, like vertex / displacement / billboard / masks
							 // ----------
	};

	class MaterialUnlit : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		void prepareMaterial() override;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialUnlit() = default;
		MaterialUnlit(const MaterialUnlit&) = delete;
		MaterialUnlit(MaterialUnlit&&) = delete;
		MaterialUnlit& operator=(const MaterialUnlit&) = delete;
		MaterialUnlit& operator=(MaterialUnlit&&) = delete;
		~MaterialUnlit() override = default;

		static void init();

		/*template <typename T = rawrbox::VertexData, typename P = rawrbox::MaterialBaseUniforms>
		void bindBaseUniforms(const rawrbox::Mesh<T>& mesh, Diligent::MapHelper<P>& helper) {
			rawrbox::MaterialBase::bindBaseUniforms<T, P>(mesh, helper); // Bind camera
		}*/

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialUnlitUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialUnlitUniforms>(mesh, CBConstants);
			// ------------
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();
			this->prepareMaterial();

			rawrbox::TextureBase* textureColor = rawrbox::WHITE_TEXTURE.get();
			rawrbox::TextureBase* textureDisplacement = rawrbox::BLACK_TEXTURE.get();

			if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
				mesh.texture->update(); // Update texture
				textureColor = mesh.texture;
			}

			if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
				mesh.displacementTexture->update(); // Update texture
				textureDisplacement = mesh.displacementTexture;
			}

			auto handle = textureColor->getHandle();
			handle->SetSampler(textureColor->getSampler());

			this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(handle);
			this->_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(textureDisplacement->getHandle());
		}

		void bindShaderResources() override;
	};
} // namespace rawrbox
