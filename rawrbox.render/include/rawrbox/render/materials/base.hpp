#pragma once

#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/models/mesh.hpp>
#include <rawrbox/render/models/vertex.hpp>
#include <rawrbox/utils/logger.hpp>

#include <InputLayout.h>
#include <PipelineState.h>

namespace rawrbox {
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

		std::pair<bool, rawrbox::BindlessPixelBuffer> _lastPixelBuffer = {false, {}};
		std::pair<bool, rawrbox::BindlessVertexBuffer> _lastVertexBuffer = {false, {}};
		std::pair<bool, rawrbox::BindlessVertexSkinnedBuffer> _lastSkinnedVertexBuffer = {false, {}};

		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Material");

	public:
		using vertexBufferType = rawrbox::VertexData;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		virtual void init() = 0;

		virtual void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::ShaderMacroHelper helper = {});
		virtual void setupPipelines(const std::string& id);
		virtual void resetUniformBinds();

		template <typename T = rawrbox::VertexData>
		rawrbox::BindlessVertexBuffer bindBaseUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::Vector4f data = {mesh.billboard ? 1.F : 0.F, mesh.vertexSnapPower, 0, 0};

			auto dTexture = mesh.textures.displacement;
			if (dTexture != nullptr) {
				data.z = dTexture->getTextureID();
				data.w = mesh.textures.displacementPower;
			}

			return {
			    mesh.color,
			    data};
		}

		template <typename T = rawrbox::VertexData>
		rawrbox::BindlessPixelBuffer bindBasePixelUniforms(const rawrbox::Mesh<T>& mesh) {
			return {
			    mesh.textures.getPixelIDs(),
			    mesh.textures.getData()};
		}

		template <typename T = rawrbox::VertexData>
		rawrbox::BindlessVertexSkinnedBuffer bindBaseVertexSkinnedUniforms(const rawrbox::Mesh<T>& mesh) {
			return {
			    mesh.boneTransforms};
		}

		template <typename T = rawrbox::VertexData>
		bool bindPixelUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessPixelBuffer buff = this->bindBasePixelUniforms<T>(mesh);
			if (this->_lastPixelBuffer.first && buff == this->_lastPixelBuffer.second) return false;
			this->_lastPixelBuffer = {true, buff};

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessPixelBuffer> PixelConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferPixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				std::memcpy(PixelConstants, &this->_lastPixelBuffer.second, sizeof(rawrbox::BindlessPixelBuffer));
			}
			// -----------

			return true;
		}

		template <typename T = rawrbox::VertexData>
		bool bindVertexUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessVertexBuffer buff = this->bindBaseUniforms<T>(mesh);
			if (this->_lastVertexBuffer.first && buff == this->_lastVertexBuffer.second) return false;
			this->_lastVertexBuffer = {true, buff};

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessVertexBuffer> VertexConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferVertex, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				std::memcpy(VertexConstants, &this->_lastVertexBuffer.second, sizeof(rawrbox::BindlessVertexBuffer));
			}
			// -----------

			return true;
		}

		template <typename T = rawrbox::VertexData>
		bool bindVertexSkinnedUniforms(const rawrbox::Mesh<T>& /*mesh*/) { return false; }

		template <typename T = rawrbox::VertexData>
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			if (this->_base == nullptr) throw this->_logger->error("Material not initialized!");
			auto context = rawrbox::RENDERER->context();

			if (mesh.wireframe) {
				if (this->_line == nullptr) throw this->_logger->error("Wireframe not supported on material");
				context->SetPipelineState(this->_wireframe);
			} else if (mesh.lineMode) {
				if (this->_line == nullptr) throw this->_logger->error("Line not supported on material");
				context->SetPipelineState(this->_line);
			} else {
				if (mesh.culling == Diligent::CULL_MODE_NONE) {
					if (this->_cullnone == nullptr) throw this->_logger->error("Disabled cull not supported on material");
					if (mesh.alphaBlend && this->_cullnone_alpha == nullptr) throw this->_logger->error("Disabled alpha cull not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullnone_alpha : this->_cullnone);
				} else if (mesh.culling == Diligent::CULL_MODE_BACK) {
					if (this->_cullback == nullptr) throw this->_logger->error("Cull back not supported on material");
					if (mesh.alphaBlend && this->_cullback_alpha == nullptr) throw this->_logger->error("Cull back alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_cullback_alpha : this->_cullback);
				} else {
					if (mesh.alphaBlend && this->_base_alpha == nullptr) throw this->_logger->error("Alpha not supported on material");
					context->SetPipelineState(mesh.alphaBlend ? this->_base_alpha : this->_base);
				}
			}
		}
	};
} // namespace rawrbox
