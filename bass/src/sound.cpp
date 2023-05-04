#include <rawrBox/bass/sound.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

#include <memory>

namespace rawrBox {
	std::shared_ptr<rawrBox::SoundInstance> Sound::createInstance() {
		if (sample == 0) return std::make_shared<SoundInstance>();
		return std::make_shared<SoundInstance>(sample, isStream, flags);
	}

	Sound::~Sound() {
		if (sample == 0) return;
		if (flags & SoundFlags::BEAT_DETECTION) BASS_FX_BPM_BeatFree(sample);

		if (isStream) {
			BASS_StreamFree(sample);
		} else {
			BASS_SampleFree(sample);
		}
	}
} // namespace rawrBox
