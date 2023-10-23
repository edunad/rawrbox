
#include <rawrbox/render/post_process/dither.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	Dither::Dither(rawrbox::DITHER_MODE dither) : _mode(dither) {
		this->_settings.g_DitherMode = static_cast<float>(dither);
	}

	Dither::~Dither() {
		RAWRBOX_DESTROY(this->_uniforms);
	}

	void Dither::updateUniforms() {
		if (this->_uniforms == nullptr) return;

		auto context = rawrbox::render::RENDERER->context();
		context->UpdateBuffer(this->_uniforms, 0, sizeof(rawrbox::DitherSetings), &this->_settings, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}

	void Dither::setIntensity(float in) {
		this->_settings.g_DitherIntensity = in * 0.01F;
		this->updateUniforms();
	}

	void Dither::setDepth(float dep) {
		this->_settings.g_DitherDepth = dep;
		this->updateUniforms();
	}

	void Dither::setColorDepth(float dep) {
		this->_settings.g_DitherColorDepth = std::pow(2, dep);
		this->updateUniforms();
	}

	void Dither::setThreshold(float th) {
		this->_settings.g_DitherThreshold = th;
		this->updateUniforms();
	}

	void Dither::setMode(rawrbox::DITHER_MODE mode) {
		this->_settings.g_DitherMode = static_cast<float>(mode);
		this->updateUniforms();
	}

	void Dither::upload() {
		auto size = rawrbox::render::RENDERER->getSize().cast<float>();
		this->_settings.g_ScreenSize = {0, 0, size.x, size.y};

		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::PostProcess::Dither::Uniforms";
		CBDesc.Size = sizeof(rawrbox::DitherSetings);
		CBDesc.Usage = Diligent::USAGE_DEFAULT;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;

		Diligent::BufferData data;
		data.DataSize = CBDesc.Size;
		data.pData = &this->_settings;
		rawrbox::render::RENDERER->device()->CreateBuffer(CBDesc, &data, &this->_uniforms);
		// ------------

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.depthFormat = false;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "post_process_base.vsh";
		settings.pPS = "dither.psh";
		settings.uniforms = {{Diligent::SHADER_TYPE_PIXEL, this->_uniforms}};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		this->_pipeline = rawrbox::PipelineUtils::createPipelines("PostProcess::Dither", "PostProcess::Dither", settings);
		this->_bind = rawrbox::PipelineUtils::getBind("PostProcess::Dither");
	}

	void Dither::applyEffect(Diligent::ITextureView* texture) {
		auto context = rawrbox::render::RENDERER->context();
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
