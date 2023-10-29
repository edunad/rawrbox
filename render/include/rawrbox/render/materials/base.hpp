#pragma once

#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/models/vertex.hpp>

#include <Graphics/GraphicsEngine/interface/InputLayout.h>
#include <Graphics/GraphicsEngine/interface/PipelineState.h>
#include <fmt/format.h>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	struct MaterialBaseUniforms {
		//  CAMERA -----
		rawrbox::Matrix4x4 _gModel;
		rawrbox::Matrix4x4 _gViewProj;
		rawrbox::Matrix4x4 _gInvView;
		rawrbox::Matrix4x4 _gWorldViewModel;
		rawrbox::Vector4f _gScreenSize;
		//  --------

		// OTHER ----
		rawrbox::Colorf _gColorOverride;
		rawrbox::Vector4f _gTextureFlags;
		std::array<rawrbox::Vector4f, 4> _gData;
		//  ----------
	};

	class MaterialBase {
		static Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

	protected:
		Diligent::IPipelineState* _base = nullptr;
		Diligent::IPipelineState* _base_alpha = nullptr;
		Diligent::IPipelineState* _line = nullptr;
		Diligent::IPipelineState* _cullback = nullptr;
		Diligent::IPipelineState* _cullback_alpha = nullptr;
		Diligent::IPipelineState* _wireframe = nullptr;
		Diligent::IPipelineState* _cullnone = nullptr;
		Diligent::IPipelineState* _cullnone_alpha = nullptr;

		Diligent::IShaderResourceBinding* _bind = nullptr;

		virtual void prepareMaterial();

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		static void init();

		template <typename T = rawrbox::VertexData, typename P = MaterialBaseUniforms>
		void bindBaseUniforms(const rawrbox::Mesh<T>& mesh, Diligent::MapHelper<P>& helper) {
			auto renderer = rawrbox::RENDERER;

			auto size = renderer->getSize().cast<float>();
			auto tTransform = rawrbox::TRANSFORM.transpose();
			auto tProj = renderer->camera()->getProjMtx().transpose();
			auto tView = renderer->camera()->getViewMtx().transpose();
			auto tInvView = renderer->camera()->getViewMtx();
			tInvView.inverse();

			auto tWorldView = renderer->camera()->getProjViewMtx().transpose();

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

			*helper = {
			    // CAMERA -------
			    tTransform,
			    tView * tProj,
			    tInvView,
			    tTransform * tWorldView,
			    size,
			    // --------------
			    mesh.color,
			    mesh.texture == nullptr ? rawrbox::Vector4f() : mesh.texture->getData(),
			    data};
			// ----------------------------
		}

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& mesh) {
			auto renderer = rawrbox::RENDERER;
			auto context = renderer->context();

			// SETUP UNIFORMS ----------------------------
			Diligent::MapHelper<rawrbox::MaterialBaseUniforms> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			this->bindBaseUniforms<T, rawrbox::MaterialBaseUniforms>(mesh, CBConstants);
			// ------------
		}

		template <typename T = rawrbox::VertexData>
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			auto context = rawrbox::RENDERER->context();

			if (mesh.wireframe) {
				if (this->_line == nullptr) throw std::runtime_error("[RawrBox-Material] Wireframe not supported on material");
				context->SetPipelineState(this->_wireframe);
			} else if (mesh.lineMode) {
				if (this->_line == nullptr) throw std::runtime_error("[RawrBox-Material] Line not supported on material");
				context->SetPipelineState(this->_line);
			} else {
				if (mesh.culling == Diligent::CULL_MODE_NONE) {
					if (this->_cullnone == nullptr) throw std::runtime_error("[RawrBox-Material] Disabled cull not supported on material");
					if (this->_cullnone_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Disabled alpha cull not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullnone_alpha : this->_cullnone);
				} else if (mesh.culling == Diligent::CULL_MODE_BACK) {
					if (this->_cullback == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back not supported on material");
					if (this->_cullback_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullback_alpha : this->_cullback);
				} else {
					if (this->_base_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_base_alpha : this->_base);
				}
			}
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

		virtual void bindShaderResources();
	};
} // namespace rawrbox
