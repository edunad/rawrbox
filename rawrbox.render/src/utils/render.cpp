#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/render/utils/render.hpp>

namespace rawrbox {
	// PROTECTED ----
	Diligent::IPipelineState* RenderUtils::_pipe = nullptr;
	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> RenderUtils::_logger = std::make_unique<rawrbox::Logger>("RawrBox-RenderUtils");
	// -------------
	// -------------

	void RenderUtils::init() {
		if (_pipe != nullptr) RAWRBOX_CRITICAL("Pipeline already initialized!");

		rawrbox::PipeSettings settings;
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.pVS = "quad.vsh";
		settings.pPS = "quad.psh";
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;              // Disable depth
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; // Set topology
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.signatures = {rawrbox::BindlessManager::signature}; // Use bindless

		_pipe = rawrbox::PipelineUtils::createPipeline("Utils::QUAD", settings);
		_logger->info("Initializing");
	}

	void RenderUtils::renderQUAD(const rawrbox::TextureBase& texture) {
		auto* context = rawrbox::RENDERER->context();
		context->SetPipelineState(_pipe);

		// SETUP VERTEX UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessVertexBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferVertex, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (CBConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the vertex constants buffer!");
		}
		// -----------

		// SETUP PIXEL UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessPixelBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferPixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			if (CBConstants == nullptr) RAWRBOX_CRITICAL("Failed to map the pixel constants buffer!");

			CBConstants->textureIDs = {texture.getTextureID(), 0, 0, 0};
		}
		// -----------

		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4; // QUAD
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
