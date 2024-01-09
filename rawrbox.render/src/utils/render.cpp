#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/render/utils/render.hpp>

namespace rawrbox {
	Diligent::IPipelineState* RenderUtils::_pipe = nullptr;

	void RenderUtils::init() {
		if (_pipe != nullptr) throw std::runtime_error("[RawrBox-RenderUtils] Pipeline already initialized!");

		rawrbox::PipeSettings settings;
		settings.pVS = "rt.vsh";
		settings.pPS = "rt.psh";
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.depthWrite = false;
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;              // Disable depth
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; // Disable cull
		settings.signature = rawrbox::BindlessManager::signature;

		_pipe = rawrbox::PipelineUtils::createPipeline("Utils::QUAD", settings);
	}

	void RenderUtils::renderQUAD(rawrbox::TextureBase* texture) {
		if (_pipe == nullptr) RenderUtils::init(); // Upload if not uploaded before

		auto context = rawrbox::RENDERER->context();
		context->SetPipelineState(_pipe);

		// SETUP VERTEX UNIFORMS ----------------------------
		/*{
			Diligent::MapHelper<rawrbox::BindlessVertexBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferVertex, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		}*/
		// -----------

		// SETUP PIXEL UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::BindlessPixelBuffer> CBConstants(context, rawrbox::BindlessManager::signatureBufferPixel, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			CBConstants->textureIDs.x = texture->getTextureID();
		}
		// -----------

		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4; // QUAD
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL | Diligent::DRAW_FLAG_DYNAMIC_RESOURCE_BUFFERS_INTACT;
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
