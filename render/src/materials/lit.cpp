#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialLit::_uniforms;
	// ----------------

	void MaterialLit::init() {
		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialLit::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialLitUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		Diligent::ShaderMacroHelper macro;
		macro.AddShaderMacro("CLUSTERS_X", rawrbox::CLUSTERS_X);
		macro.AddShaderMacro("CLUSTERS_Y", rawrbox::CLUSTERS_Y);
		macro.AddShaderMacro("CLUSTERS_Z", rawrbox::CLUSTERS_Z);

		rawrbox::PipeSettings settings;
		settings.pVS = "unlit.vsh";
		settings.pPS = "unlit.psh";
		settings.macros = macro;
		settings.layout = rawrbox::VertexNormData::vLayout();
		settings.immutableSamplers = {false, true};
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		settings.fill = Diligent::FILL_MODE_SOLID;
		rawrbox::PipelineUtils::createPipeline("Model::Lit", "Model::Lit", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Lit::Alpha", "Model::Lit", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline("Model::Lit::CullBack", "Model::Lit", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Lit::CullBack::Alpha", "Model::Lit", settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline("Model::Lit::CullNone", "Model::Lit", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Lit::CullNone::Alpha", "Model::Lit", settings);

#ifdef _DEBUG
		rawrbox::PipeSettings debugSettings;

		debugSettings.layout = rawrbox::VertexNormData::vLayout();
		debugSettings.macros = macro;
		debugSettings.pVS = "lit_debug.vsh";
		debugSettings.pPS = "cluster_debug_z.psh";
		debugSettings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}, {Diligent::SHADER_TYPE_PIXEL, _uniforms}};

		rawrbox::PipelineUtils::createPipeline("Model::Lit::Debug::ZCluster", "Model::Lit::Debug", debugSettings);
#endif
		// -----
	}

	void MaterialLit::prepareMaterial() {
		// Not a fan, but had to move it away from static, since we want to override them
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("Model::Lit");
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline("Model::Lit::Alpha");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline("Model::Lit::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline("Model::Lit::CullBack::Alpha");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline("Model::Lit::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline("Model::Lit::CullNone::Alpha");

#ifdef _DEBUG
		if (this->_debug_z == nullptr) this->_debug_z = rawrbox::PipelineUtils::getPipeline("Model::Lit::Debug::ZCluster");
		if (this->_bind_debug == nullptr) this->_bind_debug = rawrbox::PipelineUtils::getBind("Model::Lit::Debug");
#endif

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Lit");
	}

	void MaterialLit::bindShaderResources() {
		auto context = rawrbox::RENDERER->context();
		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] Bind not set!");

#ifdef _DEBUG
		if (rawrbox::RENDERER->DEBUG_LEVEL != 0) {
			context->CommitShaderResources(this->_bind_debug, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			return;
		}
#endif

		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
