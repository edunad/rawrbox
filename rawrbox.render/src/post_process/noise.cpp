
#include <rawrbox/render/post_process/noise.hpp>
#include <rawrbox/render/static.hpp>

#include <bit>

namespace rawrbox {
	PostProcessNoise::PostProcessNoise(float intensity) {
		this->setIntensity(intensity);
	}

	void PostProcessNoise::setIntensity(float in) {
		this->_data[0].x = in; // NoiseIntensity
	}

	void PostProcessNoise::init() {
		rawrbox::PostProcessBase::init();

		this->_pipeline = createPipeline("PostProcess::Noise", "post_process/noise.psh");
	}

	void PostProcessNoise::applyEffect(const rawrbox::TextureBase& source, const rawrbox::TextureRender& target, uint32_t depthID) {
		this->_data[0].y = std::bit_cast<float>(static_cast<uint32_t>(rawrbox::FRAME));
		rawrbox::PostProcessBase::applyEffect(source, target, depthID);
	}

} // namespace rawrbox