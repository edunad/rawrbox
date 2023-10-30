
#include <rawrbox/render/post_process/dither.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	PostProcessDither::PostProcessDither(rawrbox::DITHER_MODE dither) : _mode(dither) {
		this->_settings.g_DitherMode = static_cast<float>(dither);
	}

	PostProcessDither::~PostProcessDither() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void PostProcessDither::updateUniforms() {
		if (this->_uniforms == nullptr) return;

		auto context = rawrbox::RENDERER->context();
		context->UpdateBuffer(this->_uniforms, 0, sizeof(rawrbox::DitherSettings), &this->_settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void PostProcessDither::setIntensity(float in) {
		this->_settings.g_DitherIntensity = in * 0.01F;
		this->updateUniforms();
	}

	void PostProcessDither::setDepth(float dep) {
		this->_settings.g_DitherDepth = dep;
		this->updateUniforms();
	}

	void PostProcessDither::setColorDepth(float dep) {
		this->_settings.g_DitherColorDepth = std::pow(2, dep);
		this->updateUniforms();
	}

	void PostProcessDither::setThreshold(float th) {
		this->_settings.g_DitherThreshold = th;
		this->updateUniforms();
	}

	void PostProcessDither::setMode(rawrbox::DITHER_MODE mode) {
		this->_settings.g_DitherMode = static_cast<float>(mode);
		this->updateUniforms();
	}

	void PostProcessDither::upload() {
		auto size = rawrbox::RENDERER->getSize().cast<float>();
		this->_settings.g_ScreenSize = {0, 0, size.x, size.y};

		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::PostProcess::Dither::Uniforms";
		CBDesc.Size = sizeof(rawrbox::DitherSettings);
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
		settings.pPS = "dither.psh";
		settings.immutableSamplers = {true};
		settings.uniforms = {{Diligent::SHADER_TYPE_PIXEL, this->_uniforms}};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		this->_pipeline = rawrbox::PipelineUtils::createPipelines("PostProcess::Dither", "PostProcess::Dither", settings);
		this->_bind = rawrbox::PipelineUtils::getBind("PostProcess::Dither");
	}

	void PostProcessDither::applyEffect(Diligent::ITextureView* texture) {
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
