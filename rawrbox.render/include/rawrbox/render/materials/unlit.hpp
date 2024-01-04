#pragma once

#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/materials/base.hpp>

namespace rawrbox {
	class MaterialUnlit : public rawrbox::MaterialBase {
		static bool _built;

	protected:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialUnlit() = default;
		MaterialUnlit(const MaterialUnlit&) = delete;
		MaterialUnlit(MaterialUnlit&&) = delete;
		MaterialUnlit& operator=(const MaterialUnlit&) = delete;
		MaterialUnlit& operator=(MaterialUnlit&&) = delete;
		~MaterialUnlit() override = default;

		void init() override;
		void createUniforms() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::IBuffer* uniforms, Diligent::IBuffer* pixelUniforms = nullptr, Diligent::ShaderMacroHelper helper = {}) override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialBaseUniforms>(mesh, CBConstants);
			// ------------
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialUnlit] Material not bound, did you call 'init'?");

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
			// this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_DecalTexture")->Set(rawrbox::DECALS::getAtlas()->getHandle());

			this->_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement")->Set(textureDisplacement->getHandle());
		}

		void bindShaderResources() override;
	};
} // namespace rawrbox
