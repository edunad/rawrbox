
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	PostProcessNoise::PostProcessNoise(float intensity) {
		this->_settings.g_NoiseIntensity = intensity;
	}

	PostProcessNoise::~PostProcessNoise() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void PostProcessNoise::setIntensity(float in) {
		this->_settings.g_NoiseIntensity = in;
	}

	void PostProcessNoise::upload() {
		auto size = rawrbox::RENDERER->getSize().cast<float>();
		this->_settings.g_ScreenSize = {0, 0, size.x, size.y};

		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::PostProcess::Noise::Uniforms";
		CBDesc.Size = sizeof(rawrbox::NoiseSettings);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		// ------------

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.depthFormat = false;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "post_process_base.vsh";
		settings.pPS = "noise.psh";
		settings.bind = "PostProcess::Noise";
		settings.immutableSamplers = {{Diligent::SHADER_TYPE_PIXEL, "g_Texture"}};
		settings.uniforms = {{Diligent::SHADER_TYPE_PIXEL, this->_uniforms}};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Noise", settings);
		this->_bind = rawrbox::PipelineUtils::getBind("PostProcess::Noise");
	}

	void PostProcessNoise::applyEffect(Diligent::ITextureView* texture) {
		auto context = rawrbox::RENDERER->context();

		// Bind uniforms ---
		this->_settings.g_NoiseTimer = static_cast<float>(rawrbox::FRAME);

		Diligent::MapHelper<rawrbox::NoiseSettings> CBConstants(context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
		*CBConstants = this->_settings;
		// ------------

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
