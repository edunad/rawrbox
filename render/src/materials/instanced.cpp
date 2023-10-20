#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	std::unordered_map<std::string, Diligent::RefCntAutoPtr<Diligent::IPipelineState>> MaterialInstanced::_pipelines = {};
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialInstanced::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> MaterialInstanced::_SRB;
	// ----------------

	void MaterialInstanced::init() {
		/*rawrbox::MaterialBase::createUniformBuffer();

		// SHADERS ----
		Diligent::RefCntAutoPtr<Diligent::IShader> pVS;
		Diligent::RefCntAutoPtr<Diligent::IShader> pPS;

		rawrbox::PipelineUtils::compileShader("unlit_instanced.vsh", Diligent::SHADER_TYPE_VERTEX, &pVS);
		rawrbox::PipelineUtils::compileShader("unlit.psh", Diligent::SHADER_TYPE_PIXEL, &pPS); // TODO: STORE ALL GENERATED SHADERS?
		// -------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = pVS;
		settings.pPS = pPS;
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = vLayout().first;
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};

		rawrbox::PipelineUtils::createPipelines("Model::Instanced", settings, &_pipelines["base"]);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		rawrbox::PipelineUtils::createPipelines("Model::Instanced::CullBack", settings, &_pipelines["back-cull"]);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipelines("Model::Instanced::CullNone", settings, &_pipelines["no-cull"]);

		for (auto& pipe : _pipelines) {
			pipe.second->GetStaticVariableByName(Diligent::SHADER_TYPE_VERTEX, "Constants")->Set(_uniforms);
			pipe.second->CreateShaderResourceBinding(&_SRB, true);
		}*/
		// -----
	}

	void MaterialInstanced::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::RENDERER->context;

		if (mesh.culling == Diligent::CULL_MODE_NONE) {
			context->SetPipelineState(this->_pipelines["no-cull"]);
		} else {
			context->SetPipelineState(mesh.culling == Diligent::CULL_MODE_FRONT ? this->_pipelines["base"] : this->_pipelines["back-cull"]);
		}
	}

	uint32_t MaterialInstanced::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::INSTANCED;
	}

	const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> MaterialInstanced::vLayout() {
		return {rawrbox::VertexData::vLayout(true), rawrbox::VertexData::vLayoutSize()};
	}
} // namespace rawrbox
