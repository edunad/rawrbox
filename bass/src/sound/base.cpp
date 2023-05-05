#include <rawrBox/bass/sound/base.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

#include <memory>
#include <stdexcept>

namespace rawrBox {
	SoundBase::SoundBase(uint32_t sample, uint32_t fx, uint32_t flags, bool stream) : _sample(sample), _fxSample(fx), _flags(flags), _isStream(stream) {}
	SoundBase::~SoundBase() {
		if (!this->isValid()) return;
		if (this->_flags & SoundFlags::BEAT_DETECTION) BASS_FX_BPM_BeatFree(this->_sample);

		if (this->_isStream) {
			BASS_StreamFree(this->_sample);
		} else {
			BASS_SampleFree(this->_sample);
		}
	}

	std::shared_ptr<rawrBox::SoundInstance> SoundBase::createInstance() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-BASS] Sound sample not valid!");
		return std::make_shared<SoundInstance>(this->_sample, this->_isStream, this->_flags);
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

} // namespace rawrBox
