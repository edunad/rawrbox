
#include <rawrbox/render/post_process/fog.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	PostProcessFog::PostProcessFog() {
		this->_data[0] = {0.2F, 0.2F, 0.2F, 1.F};
		this->_data[1] = {static_cast<float>(rawrbox::FOG_TYPE::EXP), 0.8F, 1.0F, 0};
	}

	void PostProcessFog::setColor(const rawrbox::Colorf& color) {
		this->_data[0] = color.array(); // Color
	}

	void PostProcessFog::setType(rawrbox::FOG_TYPE type) {
		this->_data[1].x = static_cast<float>(type); // Fog type
	}

	void PostProcessFog::setEnd(float end) {
		this->_data[1].y = end; // Fog end
	}

	void PostProcessFog::setDensity(float density) {
		this->_data[1].z = density; // Fog Density
	}

	void PostProcessFog::init() {
		rawrbox::PostProcessBase::init();

		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "fog.psh";
		settings.signatures = {rawrbox::BindlessManager::signature};

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Fog", settings);
	}
} // namespace rawrbox
