
#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {

	// STATIC DATA ----
	bool MaterialUnlit::_built = false;
	// ----------------

	void MaterialUnlit::init() {
		const std::string id = "Model::Unlit";

		if (!rawrbox::MaterialUnlit::_built) {
			this->_logger->info("Building {} material..", fmt::format(fmt::fg(fmt::color::azure), id));

			this->createPipelines(id, vertexBufferType::vLayout());
			rawrbox::MaterialUnlit::_built = true;
		}

		this->setupPipelines(id);
	}

	void MaterialUnlit::createPipelines(const std::string& id, const std::vector<Diligent::LayoutElement>& layout, const Diligent::ShaderMacroHelper& helper) {
		// PIPELINE ----
		rawrbox::PipeSettings settings;
		settings.pVS = "unlit.vsh";
		settings.pPS = "unlit.psh";
		settings.cull = Diligent::CULL_MODE_FRONT;
		settings.macros = helper;
		settings.layout = layout;
		settings.renderTargets = 2;                               // COLOR + GPUPick
		settings.signature = rawrbox::BindlessManager::signature; // Use bindless

		auto* cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");
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
} // namespace rawrbox
