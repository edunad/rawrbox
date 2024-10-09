#include <rawrbox/bass/sound/base.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

#include <memory>

namespace rawrbox {
	SoundBase::SoundBase(uint32_t sample, uint32_t fx, uint32_t flags, bool stream) : _sample(sample), _fxSample(fx), _flags(flags), _isStream(stream) {}
	SoundBase::~SoundBase() {
		this->_instances.clear();

		if (!this->isValid()) {
			if ((this->_flags & SoundFlags::BEAT_DETECTION) != 0U) BASS_FX_BPM_BeatFree(this->_sample);

			if (this->_isStream) {
				BASS_StreamFree(this->_sample);
			} else {
				BASS_SampleFree(this->_sample);
			}
		}
	}

	std::shared_ptr<rawrbox::SoundInstance> SoundBase::createInstance() {
		if (!this->isValid()) CRITICAL_RAWRBOX("Sound sample not valid!");
		auto ptr = std::make_shared<rawrbox::SoundInstance>(this->_sample, this->_isStream, this->_flags);

		this->_instances.push_back(std::move(ptr));
		return this->_instances.front();
	}

	std::shared_ptr<rawrbox::SoundInstance> SoundBase::getInstance(size_t i) {
		if (i >= this->_instances.size()) CRITICAL_RAWRBOX("Sound instance '{}' not found!", i);
		return this->_instances[i];
	}

	bool SoundBase::isValid() const {
		return this->_sample != 0;
	}

	uint32_t SoundBase::getSample() const {
		return this->_sample;
	}

	uint32_t SoundBase::getFXSample() const {
		return this->_fxSample;
	}

} // namespace rawrbox
