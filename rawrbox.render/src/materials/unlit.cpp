
#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {

	// STATIC DATA ----
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialUnlit::_uniforms;
	Diligent::RefCntAutoPtr<Diligent::IBuffer> MaterialUnlit::_uniforms_pixel;

	bool MaterialUnlit::_built = false;
	// ----------------

	void MaterialUnlit::init() {
		const std::string id = "Model::Unlit";

		if (!this->_built) {
			fmt::print("[RawrBox-MaterialUnlit] Building material..\n");

			this->createUniforms();
			this->createPipelines(id, vertexBufferType::vLayout());

			this->_built = true;
		}

		this->setupPipelines(id);
	}

	void MaterialUnlit::createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, Diligent::ShaderMacroHelper helper) {
		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.macros = helper;
		settings.layout = layout;
		settings.signature = rawrbox::PipelineUtils::signature; // Use bindless

		auto cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered::Light");
		if (cluster != nullptr) {
			settings.macros = cluster->getClusterMacros() + helper;
		}

		rawrbox::PipelineUtils::createPipeline(id, settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::Alpha", settings);

		settings.blending = {};
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_LINE_LIST;
		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipeline(id + "::Line", settings);

		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		settings.cull = Diligent::CULL_MODE_BACK;
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack", settings);

		settings.blending = {Diligent::BLEND_FACTOR_SRC_ALPHA, Diligent::BLEND_FACTOR_INV_SRC_ALPHA};
		rawrbox::PipelineUtils::createPipeline(id + "::CullBack::Alpha", settings);

		settings.cull = Diligent::CULL_MODE_NONE;
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone::Alpha", settings);

		settings.blending = {};
		rawrbox::PipelineUtils::createPipeline(id + "::CullNone", settings);

		settings.blending = {};
		settings.fill = Diligent::FILL_MODE_WIREFRAME;
		rawrbox::PipelineUtils::createPipeline(id + "::Wireframe", settings);
		// -----
	}

	void MaterialUnlit::createUniforms() {
		if (this->_uniforms != nullptr || this->_uniforms_pixel != nullptr) return;

		// Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialUnlit::Vertex::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBaseUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms);
		}
		// ------------

		// Pixel Uniforms -------
		{
			Diligent::BufferDesc CBDesc;
			CBDesc.Name = "rawrbox::MaterialUnlit::Pixel::Uniforms";
			CBDesc.Size = sizeof(rawrbox::MaterialBasePixelUniforms);
			CBDesc.Usage = Diligent::USAGE_DYNAMIC;
			CBDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			CBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			rawrbox::RENDERER->device()->CreateBuffer(CBDesc, nullptr, &this->_uniforms_pixel);
		}
		// ------------
	}
} // namespace rawrbox
