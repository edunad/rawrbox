#include <rawrbox/render/materials/instanced.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialInstanced::_uniforms;

	Diligent::IPipelineState* MaterialInstanced::_base = nullptr;
	Diligent::IPipelineState* MaterialInstanced::_cullback = nullptr;
	Diligent::IPipelineState* MaterialInstanced::_wireframe = nullptr;
	Diligent::IPipelineState* MaterialInstanced::_cullnone = nullptr;

	Diligent::IShaderResourceBinding* MaterialInstanced::_bind = nullptr;
	// ----------------

	void MaterialInstanced::init() {

		// Uniforms -------
		Diligent::BufferDesc CBDesc;
		CBDesc.Name = "rawrbox::Material::Uniforms";
		CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
		CBDesc.Usage = Diligent::USAGE_DYNAMIC;
		CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

		rawrbox::render::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &_uniforms);
		// ------------

		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit_instanced.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.layout = vLayout().first;
		settings.resources = {
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_PIXEL, "g_Texture", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC},
		    Diligent::ShaderResourceVariableDesc{Diligent::SHADER_TYPE_VERTEX, "g_Displacement", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_DYNAMIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_VERTEX, _uniforms}};

		_base = rawrbox::PipelineUtils::createPipelines("Model::Instanced", "Model::Instanced", settings);

		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		_wireframe = rawrbox::PipelineUtils::createPipelines("Model::Instanced::Wireframe", "Model::Instanced", settings);

		settings.cull = Diligent::CULL_MODE_BACK;
		_cullback = rawrbox::PipelineUtils::createPipelines("Model::Instanced::CullBack", "Model::Instanced", settings);

		settings.fill = Diligent::FILL_MODE_SOLID;
		settings.cull = Diligent::CULL_MODE_NONE;
		_cullnone = rawrbox::PipelineUtils::createPipelines("Model::Instanced::CullNone", "Model::Instanced", settings);

		_bind = rawrbox::PipelineUtils::getBind("Model::Instanced");
		// -----
	}

	void MaterialInstanced::bindPipeline(const rawrbox::Mesh& mesh) {
		auto context = rawrbox::render::RENDERER->context();

		if (mesh.wireframe) {
			context->SetPipelineState(_wireframe);
		} else if (mesh.lineMode) {
			throw std::runtime_error("[RawrBox-Instanced] Line not supported on instancing");
		} else {
			if (mesh.culling == Diligent::CULL_MODE_NONE) {
				context->SetPipelineState(_cullnone);
			} else {
				context->SetPipelineState(mesh.culling == Diligent::CULL_MODE_FRONT ? _base : _cullback);
			}
		}
	}

	uint32_t MaterialInstanced::supports() const {
		return rawrbox::MaterialBase::supports() | rawrbox::MaterialFlags::INSTANCED;
	}

	const std::pair<std::vector<Diligent::LayoutElement>, uint32_t> MaterialInstanced::vLayout() {
		return {rawrbox::VertexData::vLayout(true), rawrbox::VertexData::vLayoutSize()};
	}
} // namespace rawrbox
