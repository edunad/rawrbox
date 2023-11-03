
#include <rawrbox/render/post_process/quick_bloom.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	PostProcessQuickBloom::PostProcessQuickBloom(float intensity) {
		this->_settings.g_BloomIntensity = intensity;
	}

	PostProcessQuickBloom::~PostProcessQuickBloom() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void PostProcessQuickBloom::updateUniforms() {
		if (this->_uniforms == nullptr) return;

		auto context = rawrbox::RENDERER->context();
		context->UpdateBuffer(this->_uniforms, 0, sizeof(rawrbox::BloomSettings), &this->_settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void PostProcessQuickBloom::setIntensity(float in) {
		this->_settings.g_BloomIntensity = in;
		this->updateUniforms();
	}

	void PostProcessQuickBloom::upload() {
		auto size = rawrbox::RENDERER->getSize().cast<float>();
		this->_settings.g_ScreenSize = {0, 0, size.x, size.y};

		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::PostProcess::QuickBloom::Uniforms";
		CBDesc.Size = sizeof(rawrbox::BloomSettings);
		CBDesc.Usage = Diligent::USAGE_DEFAULT;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;

		Diligent::BufferData data;
		data.DataSize = CBDesc.Size;
		data.pData = &this->_settings;
		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, &data, &this->_uniforms);
		// ------------

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.depthFormat = false;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "post_process_base.vsh";
		settings.pPS = "quick_bloom.psh";
		settings.immutableSamplers = {true};
		settings.uniforms = {{Diligent::SHADER_TYPE_PIXEL, this->_uniforms}};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::QuickBloom", "PostProcess::QuickBloom", settings);
		this->_bind = rawrbox::PipelineUtils::getBind("PostProcess::QuickBloom");
	}

	void PostProcessQuickBloom::applyEffect(Diligent::ITextureView* texture) {
		auto context = rawrbox::RENDERER->context();
		this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Texture")->Set(texture);

		context->SetPipelineState(this->_pipeline);
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Draw quad
		Diligent::DrawAttribs DrawAttrs;
		DrawAttrs.NumVertices = 4;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		context->Draw(DrawAttrs);
	}
} // namespace rawrbox
