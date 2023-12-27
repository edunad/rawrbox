#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/plugins/clustered_light.hpp>

namespace rawrbox {
	struct MaterialLitPixelUniforms {
		rawrbox::Vector4f g_LitData = {};
	};

	class MaterialLit : public rawrbox::MaterialBase {
		static bool _built;

	protected:
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms_pixel;

	public:
		using vertexBufferType = rawrbox::VertexNormData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override = default;

		void init() override;
		void createUniforms() override;
		void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::IBuffer* uniforms, Diligent::IBuffer* pixelUniforms = nullptr, Diligent::ShaderMacroHelper helper = {}) override;

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();
			auto camera = rawrbox::MAIN_CAMERA;

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBaseUniforms<T, rawrbox::MaterialBaseUniforms>(mesh, CBConstants);
			}

			{
				Diligent::MapHelper<rawrbox::MaterialLitPixelUniforms> CBConstants(context, this->_uniforms_pixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				CBConstants->g_LitData = {mesh.roughnessFactor, mesh.metalnessFactor, mesh.specularFactor, mesh.emissionFactor};
			} // ------------
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] Material not bound, did you call 'init'?");
			auto context = rawrbox::RENDERER->context();

			rawrbox::TextureBase* textureColor = rawrbox::WHITE_TEXTURE.get();
			rawrbox::TextureBase* textureDisplacement = rawrbox::BLACK_TEXTURE.get();
			rawrbox::TextureBase* textureNormal = rawrbox::NORMAL_TEXTURE.get();
			rawrbox::TextureBase* textureMetalRough = rawrbox::BLACK_TEXTURE.get();
			rawrbox::TextureBase* textureEmission = rawrbox::BLACK_TEXTURE.get();

			if (mesh.texture != nullptr && mesh.texture->isValid() && !mesh.wireframe) {
				mesh.texture->update(); // Update texture
				textureColor = mesh.texture;
			}

			if (mesh.displacementTexture != nullptr && mesh.displacementTexture->isValid()) {
				mesh.displacementTexture->update(); // Update texture
				textureDisplacement = mesh.displacementTexture;
			}

			if (mesh.normalTexture != nullptr && mesh.normalTexture->isValid() && !mesh.wireframe) {
				mesh.normalTexture->update(); // Update texture
				textureNormal = mesh.normalTexture;
			}

			if (mesh.roughtMetalTexture != nullptr && mesh.roughtMetalTexture->isValid() && !mesh.wireframe) {
				mesh.roughtMetalTexture->update(); // Update texture
				textureMetalRough = mesh.roughtMetalTexture;
			}

			if (mesh.emissionTexture != nullptr && mesh.emissionTexture->isValid() && !mesh.wireframe) {
				mesh.emissionTexture->update(); // Update texture
				textureEmission = mesh.emissionTexture;
			}

			auto texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture");
			if (texBind != nullptr) texBind->Set(textureColor->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Normal");
			if (texBind != nullptr) texBind->Set(textureNormal->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Emission");
			if (texBind != nullptr) texBind->Set(textureEmission->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_RoughMetal");
			if (texBind != nullptr) texBind->Set(textureMetalRough->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement");
			if (texBind != nullptr) texBind->Set(textureDisplacement->getHandle());
		}
	};
} // namespace rawrbox
