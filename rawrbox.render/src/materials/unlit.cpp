
#include <rawrbox/render/materials/unlit.hpp>

namespace rawrbox {

	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialUnlit::_uniforms;
	bool MaterialUnlit::_built = false;
	// ----------------

	void MaterialUnlit::init() {
		if (!_built) {
			fmt::print("[RawrBox-MaterialUnlit] Building material..\n");

			// Uniforms -------
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialUnlit::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
			// ------------

			// PIPELINE ----
			rawrbox::PipeSettings settings;
			settings.pVS = "unlit.vsh";
			settings.pPS = "unlit.psh";
			settings.cull = Diligent::CULL_MODE_FRONT;
			settings.layout = rawrbox::VertexData::vLayout();
			settings.immutableSamplers = {{Diligent::SHADER_TYPE_VERTEX, "g_Displacement"}};

			settings.resources = {
			    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
			    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
			settings.uniforms = {
			    {Diligent::SHADER_TYPE_VERTEX, rawrbox::MAIN_CAMERA->uniforms(), "Camera"},
			    {Diligent::SHADER_TYPE_VERTEX, _uniforms, "Constants"}};

			rawrbox::PipelineUtils::createPipeline("Model::Unlit", "Model::Unlit", settings);

			settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::Alpha", "Model::Unlit", settings);

			settings.blending = {};
			settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
			settings.cull = Diligent::CULL_MODE_NONE;
			rawrbox::PipelineUtils::createPipeline("Model::Line", "Model::Unlit", settings);

			settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			settings.cull = Diligent::CULL_MODE_BACK;
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::CullBack", "Model::Unlit", settings);

			settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::CullBack::Alpha", "Model::Unlit", settings);

			settings.cull = Diligent::CULL_MODE_NONE;
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::CullNone::Alpha", "Model::Unlit", settings);

			settings.blending = {};
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::CullNone", "Model::Unlit", settings);

			settings.blending = {};
			settings.fill = Diligent::FILL_MODE_WIREFRAME;
			rawrbox::PipelineUtils::createPipeline("Model::Unlit::Wireframe", "Model::Unlit", settings);
			// -----

			this->_built = true;
		}

		if (this->_base == nullptr) this->_base = rawrbox::PipelineUtils::getPipeline("Model::Unlit");
		if (this->_base_alpha == nullptr) this->_base_alpha = rawrbox::PipelineUtils::getPipeline("Model::Unlit::Alpha");

		if (this->_line == nullptr) this->_line = rawrbox::PipelineUtils::getPipeline("Model::Line");

		if (this->_cullback == nullptr) this->_cullback = rawrbox::PipelineUtils::getPipeline("Model::Unlit::CullBack");
		if (this->_cullback_alpha == nullptr) this->_cullback_alpha = rawrbox::PipelineUtils::getPipeline("Model::Unlit::CullBack::Alpha");

		if (this->_wireframe == nullptr) this->_wireframe = rawrbox::PipelineUtils::getPipeline("Model::Unlit::Wireframe");

		if (this->_cullnone == nullptr) this->_cullnone = rawrbox::PipelineUtils::getPipeline("Model::Unlit::CullNone");
		if (this->_cullnone_alpha == nullptr) this->_cullnone_alpha = rawrbox::PipelineUtils::getPipeline("Model::Unlit::CullNone::Alpha");

		if (this->_bind == nullptr) this->_bind = rawrbox::PipelineUtils::getBind("Model::Unlit");
	}

	void MaterialUnlit::bindShaderResources() {
		if (this->_bind == nullptr) throw std::runtime_error("[RawrBox-MaterialUnlit] Bind not set!");

		auto context = rawrbox::RENDERER->context();
		context->CommitShaderResources(this->_bind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	}
} // namespace rawrbox
