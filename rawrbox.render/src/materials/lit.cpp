/*#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialLit::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialLit::_uniforms_pixel;
	// ----------------

	void MaterialLit::init() {
		auto cluster = dynamic_cast<rawrbox::RendererCluster*>(rawrbox::RENDERER);
		if (cluster == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] This material requires the `clustered` renderer");

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialLit::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialLitUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		}

		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialLit::Uniforms::Pixel";
			CBDesc.Size = sizeof(rawrbox::MaterialLitPixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms_pixel);
		}
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "lit.vsh";
		settings.pPS = "lit.psh";
		settings.macros = cluster->getClusterMacros();
		settings.layout = rawrbox::VertexNormData::vLayout();
		settings.immutableSamplers = {{Diligent::SHADER_TYPE_VERTEX, "g_Displacement"}, {Diligent::SHADER_TYPE_PIXEL, "g_Normal"}, {Diligent::SHADER_TYPE_PIXEL, "g_Specular"}};

		settings.resources = {
		    {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Normal", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Specular", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Emission", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Lights", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_ClusterDataGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "LightConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, _uniforms_pixel, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, rawrbox::LIGHTS::uniforms, "LightConstants"},
		    {Diligent::SHADER_TYPE_PIXEL, rawrbox::LIGHTS::getBuffer(), "g_Lights"},
		    {Diligent::SHADER_TYPE_PIXEL, cluster->getDataGridBuffer(), "g_ClusterDataGrid"}};

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
		debugSettings.macros = cluster->getClusterMacros();
		debugSettings.pVS = "lit_debug.vsh";
		debugSettings.pPS = "cluster_debug_clusters.psh";

		debugSettings.resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_ClusterDataGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		debugSettings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, _uniforms_pixel, "Constants"},

		    {Diligent::SHADER_TYPE_PIXEL, cluster->getDataGridBuffer(), "g_ClusterDataGrid"}};

		rawrbox::PipelineUtils::createPipeline("Model::Lit::Debug::Cluster", "Model::Lit::Debug::Cluster", debugSettings);
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
		if (this->_debug_cluster == nullptr) this->_debug_cluster = rawrbox::PipelineUtils::getPipeline("Model::Lit::Debug::Cluster");
		if (this->_bind_debug_cluster == nullptr) this->_bind_debug_cluster = rawrbox::PipelineUtils::getBind("Model::Lit::Debug::Cluster");
#endif
		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Lit");
	}

	void MaterialLit::bindShaderResources() {
		auto context = rawrbox::RENDERER->context();
#ifdef _DEBUG
		switch (rawrbox::RENDERER->DEBUG_LEVEL) {
			case 1:
				context->CommitShaderResources(this->_bind_debug_cluster, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				return;
		}
#endif

		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] Bind not set!");
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
*/