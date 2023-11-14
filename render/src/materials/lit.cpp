#include <rawrbox/render/light/manager.hpp>
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
		Diligent::ShaderMacroHelper macro;
		macro.AddShaderMacro("CLUSTERS_X", rawrbox::CLUSTERS_X);
		macro.AddShaderMacro("CLUSTERS_Y", rawrbox::CLUSTERS_Y);
		macro.AddShaderMacro("CLUSTERS_Z", rawrbox::CLUSTERS_Z);
		macro.AddShaderMacro("MAX_LIGHTS_PER_CLUSTER", rawrbox::MAX_LIGHTS_PER_CLUSTER);

		rawrbox::PipeSettings settings;
		settings.resourceType = Diligent::SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;
		settings.pVS = "lit.vsh";
		settings.pPS = "lit.psh";
		settings.macros = macro;
		settings.layout = rawrbox::VertexNormData::vLayout();
		settings.immutableSamplers = {{Diligent::SHADER_TYPE_VERTEX, "g_Displacement"}, {Diligent::SHADER_TYPE_PIXEL, "g_Normal"}, {Diligent::SHADER_TYPE_PIXEL, "g_Specular"}};

		settings.resources = {
		    {Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Normal", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Specular", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_Emission", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_Lights", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_ClusterLightIndices", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "g_ClusterGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "LightConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, _uniforms_pixel, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, rawrbox::LIGHTS::uniforms, "LightConstants"},
		    {Diligent::SHADER_TYPE_PIXEL, cluster->getLightIndicesBuffer(), "g_ClusterLightIndices"},
		    {Diligent::SHADER_TYPE_PIXEL, cluster->getDataGridBuffer(), "g_ClusterGrid"}};

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

		debugSettings.resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		debugSettings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, _uniforms_pixel, "Constants"}};

		rawrbox::PipelineUtils::createPipeline("Model::Lit::Debug::ZCluster", "Model::Lit::Debug", debugSettings);

		debugSettings.resources = {
		    {Diligent::SHADER_TYPE_VERTEX, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_PIXEL, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_PIXEL, "g_ClusterGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		debugSettings.uniforms = {
		    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"},
		    {Diligent::SHADER_TYPE_PIXEL, _uniforms_pixel, "Constants"},

		    {Diligent::SHADER_TYPE_PIXEL, cluster->getDataGridBuffer(), "g_ClusterGrid"}};

		debugSettings.pVS = "lit_debug.vsh";
		debugSettings.pPS = "cluster_debug_light.psh";
		rawrbox::PipelineUtils::createPipeline("Model::Lit::Debug::Light", "Model::Lit::Debug::Light", debugSettings);
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
		if (this->_debug_light == nullptr) this->_debug_light = rawrbox::PipelineUtils::getPipeline("Model::Lit::Debug::Light");

		if (this->_bind_debug_z == nullptr) this->_bind_debug_z = rawrbox::PipelineUtils::getBind("Model::Lit::Debug");
		if (this->_bind_debug_light == nullptr) this->_bind_debug_light = rawrbox::PipelineUtils::getBind("Model::Lit::Debug::Light");
#endif

		if (this->_bind == nullptr) {
			this->_bind = rawrbox::PipelineUtils::getBind("Model::Lit");

			auto l = this->_bind->GetVariableByName(Diligent::SHADER_TYPE_PIXEL, "g_Lights");

			if (l != nullptr) {
				l->Set(rawrbox::LIGHTS::getBuffer());

				rawrbox::LIGHTS::onBufferResize += [this, l]() {
					if (l != nullptr) l->Set(rawrbox::LIGHTS::getBuffer());
				};
			}
		}
	}

	void MaterialLit::bindShaderResources() {
		auto context = rawrbox::RENDERER->context();
		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialLit] Bind not set!");

#ifdef _DEBUG
		switch (rawrbox::RENDERER->DEBUG_LEVEL) {
			case 1:
				context->CommitShaderResources(this->_bind_debug_z, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				return;
			case 2:
				context->CommitShaderResources(this->_bind_debug_light, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
				return;
		}
#endif

		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
