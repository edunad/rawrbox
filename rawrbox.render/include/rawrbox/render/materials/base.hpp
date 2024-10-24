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
		std::optional<rawrbox::BindlessPixelBuffer> _lastPixelBuffer = std::nullopt;
		std::optional<rawrbox::BindlessVertexBuffer> _lastVertexBuffer = std::nullopt;
		std::optional<rawrbox::BindlessVertexSkinnedBuffer> _lastSkinnedVertexBuffer = std::nullopt;

		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Material");

	public:
		using vertexBufferType = rawrbox::VertexData;

		Diligent::IPipelineState* base = nullptr;
		Diligent::IPipelineState* base_alpha = nullptr;
		Diligent::IPipelineState* line = nullptr;
		Diligent::IPipelineState* cullback = nullptr;
		Diligent::IPipelineState* cullback_alpha = nullptr;
		Diligent::IPipelineState* wireframe = nullptr;
		Diligent::IPipelineState* cullnone = nullptr;
		Diligent::IPipelineState* cullnone_alpha = nullptr;

		MaterialBase() = default;
		MaterialBase(MaterialBase&&) = delete;
		MaterialBase& operator=(MaterialBase&&) = delete;
		MaterialBase(const MaterialBase&) = delete;
		MaterialBase& operator=(const MaterialBase&) = delete;
		virtual ~MaterialBase() = default;

		virtual void init() = 0;

		virtual void createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper = {});
		virtual void setupPipelines(const std::string& id);
		virtual void resetUniformBinds();

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		rawrbox::BindlessVertexBuffer bindBaseUniforms(const rawrbox::Mesh<T>& mesh) {
			auto* texture = mesh.getTexture();
			auto* displacement = mesh.getDisplacementTexture();

			return {
			    {mesh.color.pack(), mesh.getSlice() + (texture == nullptr ? 0U : texture->getSlice()), mesh.getID(), mesh.data.billboard},
			    {mesh.data.vertexSnapPower, displacement != nullptr ? static_cast<float>(displacement->getTextureID()) : 0.F, mesh.data.displacementPower}};
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		rawrbox::BindlessPixelBuffer bindBasePixelUniforms(const rawrbox::Mesh<T>& mesh) {
			return {
			    mesh.textures.getTextureIDs(),
			    mesh.textures.getTextureData(),
			    mesh.textures.getData()};
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		rawrbox::BindlessVertexSkinnedBuffer bindBaseVertexSkinnedUniforms(const rawrbox::Mesh<T>& mesh) {
			return {
			    mesh.boneTransforms};
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		bool bindPixelUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessPixelBuffer buff = this->bindBasePixelUniforms<T>(mesh);
			if (this->_lastPixelBuffer.has_value() && buff == this->_lastPixelBuffer.value()) return false;
			this->_lastPixelBuffer = buff;

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessPixelBuffer> PixelConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferPixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				if (PixelConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the pixel constants buffer!");

				std::memcpy(PixelConstants, &this->_lastPixelBuffer.value(), sizeof(rawrbox::BindlessPixelBuffer));
			}
			// -----------

			return true;
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		bool bindVertexUniforms(const rawrbox::Mesh<T>& mesh) {
			rawrbox::BindlessVertexBuffer buff = this->bindBaseUniforms<T>(mesh);
			if (this->_lastVertexBuffer.has_value() && buff == this->_lastVertexBuffer.value()) return false;
			this->_lastVertexBuffer = buff;

			// SETUP UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::BindlessVertexBuffer> VertexConstants(rawrbox::RENDERER->context(), rawrbox::BindlessManager::signatureBufferVertex, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				if (VertexConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the vertex constants buffer!");

				std::memcpy(VertexConstants, &this->_lastVertexBuffer.value(), sizeof(rawrbox::BindlessVertexBuffer));
			}
			// -----------

			return true;
		}

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		bool bindVertexSkinnedUniforms(const rawrbox::Mesh<T>& /*mesh*/) { return false; }

		template <typename T = rawrbox::VertexData>
			requires(std::derived_from<T, rawrbox::VertexData>)
		void bindPipeline(const rawrbox::Mesh<T>& mesh) {
			if (this->base == nullptr) RAWRBOX_CRITICAL("Material not initialized!");
			auto* context = rawrbox::RENDERER->context();

			if (mesh.getWireframe()) {
				if (this->wireframe == nullptr) RAWRBOX_CRITICAL("Wireframe not supported on material");
				context->SetPipelineState(this->wireframe);
			} else if (mesh.getLineMode()) {
				if (this->line == nullptr) RAWRBOX_CRITICAL("Line not supported on material");
				context->SetPipelineState(this->line);
			} else {
				if (mesh.culling == Diligent::CULL_MODE_NONE) {
					if (this->cullnone == nullptr) RAWRBOX_CRITICAL("Disabled cull not supported on material");
					if (mesh.isTransparent() && this->cullnone_alpha == nullptr) RAWRBOX_CRITICAL("Disabled alpha cull not supported on material");
					context->SetPipelineState(mesh.isTransparent() ? this->cullnone_alpha : this->cullnone);
				} else if (mesh.culling == Diligent::CULL_MODE_BACK) {
					if (this->cullback == nullptr) RAWRBOX_CRITICAL("Cull back not supported on material");
					if (mesh.isTransparent() && this->cullback_alpha == nullptr) RAWRBOX_CRITICAL("Cull back alpha not supported on material");
					context->SetPipelineState(mesh.isTransparent() ? this->cullback_alpha : this->cullback);
				} else {
					if (mesh.isTransparent() && this->base_alpha == nullptr) RAWRBOX_CRITICAL("Alpha not supported on material");
					context->SetPipelineState(mesh.isTransparent() ? this->base_alpha : this->base);
				}
			}
		}
	};
} // namespace rawrbox
