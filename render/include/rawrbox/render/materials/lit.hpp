#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/renderers/cluster.hpp>

namespace rawrbox {

	struct MaterialLitUniforms : public rawrbox::MaterialUnlitUniforms, public rawrbox::ClusterUniforms {};

	class MaterialLit : public rawrbox::MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
#ifdef _DEBUG
		Diligent::IPipelineState* _debug_z = nullptr;
		Diligent::IShaderResourceBinding* _bind_debug = nullptr;
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

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialLitUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialLitUniforms>(mesh, CBConstants);
			// ------------

			// Bind renderer uniforms ---------
			auto cluster = dynamic_cast<rawrbox::RendererCluster*>(rawrbox::RENDERER);
			if (cluster == nullptr) return;

			cluster->bindUniforms<rawrbox::MaterialLitUniforms>(CBConstants);
			// --------------------------------
		}

		template <typename T = rawrbox::VertexData, typename P = rawrbox::MaterialBaseUniforms>
		void bindBaseUniforms(const rawrbox::Mesh<T>& mesh, Diligent::MapHelper<P>& helper) {
			rawrbox::MaterialBase::bindBaseUniforms<T, P>(mesh, helper); // Bind camera

			std::array<rawrbox::Vector4f, MAX_DATA>
			    data = {rawrbox::Vector4f{0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}, {0.F, 0.F, 0.F, 0.F}};

			if (mesh.hasData("billboard_mode")) {
				data[0] = mesh.getData("billboard_mode").data();
			}

			if (mesh.hasData("vertex_snap")) {
				data[1] = mesh.getData("vertex_snap").data();
			}

			if (mesh.hasData("displacement_strength")) {
				data[2] = mesh.getData("displacement_strength").data();
			}

			if (mesh.hasData("mask")) {
				data[3] = mesh.getData("mask").data();
			}

			(*helper)._gColorOverride = mesh.color;
			(*helper)._gTextureFlags = mesh.texture == nullptr ? rawrbox::Vector4f() : mesh.texture->getData();
			(*helper)._gData = data;
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();
			this->prepareMaterial();

#ifdef _DEBUG
			if (rawrbox::RendererBase::DEBUG_LEVEL != 0) {
				return;
			}
#endif

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

#ifdef _DEBUG
		template <typename T = rawrbox::VertexData>
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			if (rawrbox::RendererBase::DEBUG_LEVEL == 1) {
				context->SetPipelineState(this->_debug_z);
			} else {
				rawrbox::MaterialBase::bindPipeline<T>(mesh);
			}
		}
#endif

		void bindShaderResources() override;
	};

} // namespace rawrbox
