
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/post_process/dither.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

namespace rawrbox {
	PostProcessDither::PostProcessDither(rawrbox::DITHER_MODE dither) : _mode(dither) {
		// Default settings ----
		this->setMode(dither);
		this->setIntensity(12.F);
		this->setColorDepth(5.F);
		this->setThreshold(1.F);
		// ---------------------
	}

	void PostProcessDither::setIntensity(float in) {
		this->_data[0].y = in * 0.01F; // DitherIntensity
	}

	void PostProcessDither::setColorDepth(float dep) {
		this->_data[0].z = std::pow(2.F, dep); // DitherColorDepth
	}

	void PostProcessDither::setThreshold(float th) {
		this->_data[0].w = th; // DitherThreshold
	}

	void PostProcessDither::setMode(rawrbox::DITHER_MODE mode) {
		this->_data[0].x = static_cast<float>(mode); // DitherMode
	}

	void PostProcessDither::init() {
		rawrbox::PostProcessBase::init();

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "dither.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Dither", settings);
	}
} // namespace rawrbox
