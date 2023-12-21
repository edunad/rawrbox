#pragma once

#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/models/vertex.hpp>

#include <InputLayout.h>
#include <PipelineState.h>
#include <fmt/format.h>

namespace rawrbox {
	constexpr auto MAX_DATA = 4;

	struct MaterialBaseUniforms {
		// Model ----
		rawrbox::Colorf _gColorOverride = {};
		rawrbox::Vector4f _gTextureFlags = {};

		std::array<rawrbox::Vector4f, 4> _gData; // Other mesh data, like vertex / displacement / billboard / masks
							 // ----------
	};

	class MaterialBase {
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

		virtual void prepareMaterial() = 0;

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		template <typename T = rawrbox::VertexData, typename P = rawrbox::MaterialBaseUniforms>
		void bindBaseUniforms(const rawrbox::Mesh<T>& mesh, Diligent::MapHelper<P>& helper) {
			// MODEL -------
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

			helper->_gColorOverride = mesh.color;
			helper->_gTextureFlags = mesh.texture == nullptr ? rawrbox::Vector4f() : mesh.texture->getData();
			helper->_gData = data;
			// ----------------------------
		}

		template <typename T = rawrbox::VertexData>
		void bindUniforms(const rawrbox::Mesh<T>& /*mesh*/) {
			throw std::runtime_error("[RawrBox-Material] Missing implementation");
		}

		template <typename T = rawrbox::VertexData>
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-Material] Material not initialized!");
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
					if (mesh.alphaBlend && this->_cullnone_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Disabled alpha cull not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullnone_alpha : this->_cullnone);
				} else if (mesh.culling == Diligent::CULL_MODE_BACK) {
					if (this->_cullback == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back not supported on material");
					if (mesh.alphaBlend && this->_cullback_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Cull back alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullback_alpha : this->_cullback);
				} else {
					if (mesh.alphaBlend && this->_base_alpha == nullptr) throw std::runtime_error("[RawrBox-Material] Alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_base_alpha : this->_base);
				}
			}
		}

		template <typename T = rawrbox::VertexData>
		void bindTexture(const rawrbox::Mesh<T>& /*mesh*/) {
			throw std::runtime_error("[RawrBox-Material] Missing implementation");
		}

		virtual void bindShaderResources() = 0;
	};
} // namespace rawrbox
