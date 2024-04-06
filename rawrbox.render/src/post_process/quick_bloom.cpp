
#include <rawrbox/render/post_process/quick_bloom.hpp>

namespace rawrbox {
	PostProcessQuickBloom::PostProcessQuickBloom(float intensity) {
		this->setIntensity(intensity);
	}

	void PostProcessQuickBloom::setIntensity(float in) {
		this->_data[0].x = in; // BloomIntensity
	}

	void PostProcessQuickBloom::init() {
		rawrbox::PostProcessBase::init();

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "quick_bloom.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::QuickBloom", settings);
	}
} // namespace rawrbox
