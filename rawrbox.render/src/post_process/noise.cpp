
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/static.hpp>

namespace rawrbox {
	PostProcessNoise::PostProcessNoise(float intensity) {
		this->setIntensity(intensity);
	}

	void PostProcessNoise::setIntensity(float in) {
		this->_data[0].x = in; // NoiseIntensity
	}

	void PostProcessNoise::init() {
		rawrbox::PipeSettings settings;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.pVS = "quad.vsh";
		settings.pPS = "noise.psh";
		settings.signature = rawrbox::BindlessManager::signature;

		this->_pipeline = rawrbox::PipelineUtils::createPipeline("PostProcess::Noise", settings);
	}

	void PostProcessNoise::applyEffect(const rawrbox::TextureBase& texture) {
		this->_data[0].y = static_cast<float>(rawrbox::FRAME); // NoiseTimer
		rawrbox::PostProcessBase::applyEffect(texture);
	}

} // namespace rawrbox
