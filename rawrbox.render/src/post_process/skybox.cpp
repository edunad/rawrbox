
#include <rawrbox/render/post_process/skybox.hpp>

#include <bit>

namespace rawrbox {
	PostProcessSkybox::PostProcessSkybox(const rawrbox::Colorf& start, const rawrbox::Colorf& end) {
		this->setStartColor(start);
		this->setEndColor(end);

		this->setPSXBanding(64);
	}

	void PostProcessSkybox::setStartColor(const rawrbox::Colorf& col) { this->_data[0].x = std::bit_cast<float>(col.pack()); }
	void PostProcessSkybox::setEndColor(const rawrbox::Colorf& col) { this->_data[0].y = std::bit_cast<float>(col.pack()); }
	void PostProcessSkybox::setPSXBanding(uint32_t banding) { this->_data[0].z = static_cast<float>(banding); }

	void PostProcessSkybox::init() {
		rawrbox::PostProcessBase::init();

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.depthWrite = false;
		settings.depthFormat = false;
		settings.blending = {.src = Diligent::BLEND_FACTOR_UNDEFINED, .dest = Diligent::BLEND_FACTOR_UNDEFINED};
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "skybox.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Skybox", settings);
	}
} // namespace rawrbox