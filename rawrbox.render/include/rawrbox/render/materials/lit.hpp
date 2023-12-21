#pragma once
/*
#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/renderers/cluster.hpp>

namespace rawrbox {

	struct MaterialLitUniforms : public rawrbox::MaterialUnlitUniforms, public rawrbox::ClusterUniforms {};
	struct MaterialLitPixelUniforms : public rawrbox::ClusterUniforms {
		rawrbox::Vector4f g_LightGridParams = {};
		rawrbox::Vector4f g_LitData = {};
	};

	class MaterialLit : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms_pixel;

	protected:
#ifdef _DEBUG
		Diligent::IPipelineState* _debug_cluster = nullptr;
		Diligent::IShaderResourceBinding* _bind_debug_cluster = nullptr;
#endif

		void prepareMaterial() override;

	public:
		using vertexBufferType = rawrbox::VertexNormData;

		MaterialLit() = default;
		MaterialLit(MaterialLit&&) = delete;
		MaterialLit& operator=(MaterialLit&&) = delete;
		MaterialLit(const MaterialLit&) = delete;
		MaterialLit& operator=(const MaterialLit&) = delete;
		~MaterialLit() override = default;

		static void init();

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();
			auto cluster = dynamic_cast<rawrbox::RendererCluster*>(rawrbox::RENDERER);
			if (cluster == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] This material requires the `clustered` renderer");

			auto camera = rawrbox::MAIN_CAMERA;

			{
				// SETUP UNIFORMS ----------------------------
				Diligent::MapHelper<rawrbox::MaterialLitUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				this->bindBaseUniforms<T, rawrbox::MaterialLitUniforms>(mesh, CBConstants);
				// ------------

				// Bind renderer uniforms ---------
				cluster->bindUniforms<rawrbox::MaterialLitUniforms>(CBConstants);
				// --------------------------------
			}

			{
				Diligent::MapHelper<rawrbox::MaterialLitPixelUniforms> CBConstants(context, this->_uniforms_pixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

				float nearZ = camera->getZNear();
				float farZ = camera->getZFar();

				float n = std::min(farZ, nearZ);
				float f = std::max(farZ, nearZ);
				auto gLightClustersNumZz = static_cast<float>(rawrbox::CLUSTERS_Z);

				CBConstants->g_LightGridParams = {
				    gLightClustersNumZz / std::log(f / n),
				    (gLightClustersNumZz * std::log(n)) / std::log(f / n)};

				CBConstants->g_CameraPosition = camera->getPos();
				CBConstants->g_LitData = {mesh.specularShininess, mesh.emissionIntensity};

				cluster->bindUniforms<rawrbox::MaterialLitPixelUniforms>(CBConstants);
			}
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();
			this->prepareMaterial();

#ifdef _DEBUG
			if (rawrbox::RendererBase::DEBUG_LEVEL != 0) return; // Debug does not have textures
#endif

			rawrbox::TextureBase* textureColor = rawrbox::WHITE_TEXTURE.get();
			rawrbox::TextureBase* textureDisplacement = rawrbox::BLACK_TEXTURE.get();
			rawrbox::TextureBase* textureNormal = rawrbox::NORMAL_TEXTURE.get();
			rawrbox::TextureBase* textureSpecular = rawrbox::BLACK_TEXTURE.get();
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

			if (mesh.specularTexture != nullptr && mesh.specularTexture->isValid() && !mesh.wireframe) {
				mesh.specularTexture->update(); // Update texture
				textureSpecular = mesh.specularTexture;
			}

			if (mesh.emissionTexture != nullptr && mesh.emissionTexture->isValid() && !mesh.wireframe) {
				mesh.emissionTexture->update(); // Update texture
				textureEmission = mesh.emissionTexture;
			}

			auto texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture");
			if (texBind != nullptr) texBind->Set(textureColor->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Normal");
			if (texBind != nullptr) texBind->Set(textureNormal->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Specular");
			if (texBind != nullptr) texBind->Set(textureSpecular->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Emission");
			if (texBind != nullptr) texBind->Set(textureEmission->getHandle());

			texBind = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_VERTEX, "g_Displacement");
			if (texBind != nullptr) texBind->Set(textureDisplacement->getHandle());
		}

#ifdef _DEBUG
		template <typename T = rawrbox::VertexData>
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			if (rawrbox::RendererBase::DEBUG_LEVEL == 1) {
				context->SetPipelineState(this->_debug_cluster);
			} else {
				rawrbox::MaterialBase::bindPipeline<T>(mesh);
			}
		}
#endif

		void bindShaderResources() override;
	};

} // namespace rawrbox
*/
