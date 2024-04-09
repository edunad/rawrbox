
#include <rawrbox/render/post_process/skybox.hpp>

namespace rawrbox {
	PostProcessSkybox::PostProcessSkybox(const rawrbox::Colorf& start, const rawrbox::Colorf& end) {
		this->setStartColor(start);
		this->setEndColor(end);

		this->setPSXBanding(64);
	}

	void PostProcessSkybox::setStartColor(const rawrbox::Colorf& col) { this->_data[0].x = col.pack(); }
	void PostProcessSkybox::setEndColor(const rawrbox::Colorf& col) { this->_data[0].y = col.pack(); }
	void PostProcessSkybox::setPSXBanding(uint32_t banding) { this->_data[0].z = banding; }

	void PostProcessSkybox::init() {
		rawrbox::PostProcessBase::init();

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "skybox.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Skybox", settings);
	}
} // namespace rawrbox
