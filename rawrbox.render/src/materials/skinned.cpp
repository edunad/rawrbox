#include <rawrbox/render/materials/skinned.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialSkinned::_uniforms;
	// ----------------

	void MaterialSkinned::init() {
		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::MaterialSkinned::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialSkinnedUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit_skinned.vsh";
		settings.pPS = "unlit.psh";
		settings.layout = rawrbox::VertexBoneData::vLayout();
		settings.immutableSamplers = {{Diligent::SHADER_TYPE_VERTEX, "g_Displacement"}};
		settings.macros.AddShaderMacro("MAX_BONES", rawrbox::MAX_BONES_PER_MODEL);
		settings.macros.AddShaderMacro("NUM_BONES_PER_VERTEX", rawrbox::MAX_BONES_PER_VERTEX);

		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::Wireframe", "Model::Skinned", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		rawrbox::PipelineUtils::createPipeline("Model::Skinned", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::Alpha", "Model::Skinned", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.cull = Diligent::CULL_MODE_BACK;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::CullBack", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::CullBack::Alpha", "Model::Skinned", settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::CullNone", "Model::Skinned", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline("Model::Skinned::CullNone::Alpha", "Model::Skinned", settings);

		// -----
	}

	void MaterialSkinned::prepareMaterial() {
		// Not a fan, but had to move it away from static, since we want to override them
		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("Model::Skinned");
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline("Model::Skinned::Wireframe");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullBack::Alpha");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline("Model::Skinned::CullNone::Alpha");

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Skinned");
	}
} // namespace rawrbox
