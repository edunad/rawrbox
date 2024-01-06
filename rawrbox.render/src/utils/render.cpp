#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>
#include <rawrbox/render/utils/render.hpp>

namespace rawrbox {
	Diligent::IPipelineState* RenderUtils::_pipe = nullptr;
	Diligent::IShaderResourceBinding* RenderUtils::_SRB = nullptr;

	void RenderUtils::init() {
		if (_pipe != nullptr || _SRB != nullptr) throw std::runtime_error("[RawrBox-RenderUtils] Pipeline already initialized!");

		rawrbox::PipeSettings settings;
		settings.pVS = "rt.vsh";
		settings.pPS = "rt.psh";
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.depthWrite = false;
		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		settings.immutableSamplers = {{Diligent::SHADER_TYPE_PIXEL, "g_Texture"}};
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;              // Disable depth
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; // Disable cull
		settings.bind = "Utils::QUAD";
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}};

		_pipe = rawrbox::PipelineUtils::createPipeline("Utils::QUAD", settings);
		_SRB = rawrbox::PipelineUtils::getBind("Utils::QUAD");
	}

	void RenderUtils::renderQUAD(Diligent::ITextureView* texture, bool transition) {
		if (_pipe == nullptr) {
			RenderUtils::init(); // Upload if not uploaded before
		}

		auto context = rawrbox::RENDERER->context();
		_SRB->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture);

		context->SetPipelineState(_pipe);
		context->CommitShaderResources(_SRB, transition ? Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION : Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		Diligent::DrawAttribs drawAttrs;
		drawAttrs.NumVertices = 4; // QUAD
		drawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL;
		context->Draw(drawAttrs);
	}
} // namespace rawrbox
