#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/sound/flags.hpp>
#include <rawrbox/bass/sound/instance.hpp>
#include <rawrbox/bass/utils/bass.hpp>

#include <bass.h>
#include <bass_fx.h>
#include <fmt/printf.h>

namespace rawrbox {
	SoundInstance::SoundInstance(uint32_t audioSample, bool isStream, uint32_t flags) : _sample(audioSample), _flags(flags), _stream(isStream) {
		rawrbox::BASS::onBEAT.add(std::to_string(this->_sample), [this](std::pair<uint32_t, double> data) {
			if (this->_sample == data.first || this->_channel == data.first) this->onBEAT(data.second);
		});

		rawrbox::BASS::onBPM.add(std::to_string(this->_sample), [this](std::pair<uint32_t, float> data) {
			if (this->_sample == data.first || this->_channel == data.first) this->onBPM(data.second);
		});

		rawrbox::BASS::onSoundEnd.add(std::to_string(this->_sample), [this](uint32_t sample) {
			if (this->_sample == sample || this->_channel == sample) this->onEnd();
		});
	}

	SoundInstance::~SoundInstance() {
		if (!this->isCreated()) return;

		BASS_ChannelStop(this->_channel);
		if (!this->_stream) {
			BASS_ChannelFree(this->_channel);
		} else {
			BASS_StreamFree(this->_channel);
		}

		// CLEANUP CALLBACKS
		rawrbox::BASS::onBEAT.remove(std::to_string(this->_sample));
		rawrbox::BASS::onSoundEnd.remove(std::to_string(this->_sample));
	}

	uint32_t SoundInstance::getNextAvailableChannel() const {
		if (!isValid()) return 0;
		if (this->_stream) return this->_sample; // streams only need 1 channel instance

		// no slots free, so lets kill the furthest away or oldest if we're over the 5 channels limit
		auto newInstance = BASS_SampleGetChannel(this->_sample, this->is3D() ? BASS_SAMPLE_OVER_DIST : BASS_SAMPLE_OVER_POS);
		rawrbox::BASSUtils::checkBASSError();

		return newInstance;
	}

	void SoundInstance::play() {
		if (this->isCreated() && this->isPaused()) {
			BASS_ChannelPlay(this->_channel, 0);
			rawrbox::BASSUtils::checkBASSError();
			return;
		}

		this->_channel = this->getNextAvailableChannel();
		if (this->_channel == 0) return;

		// re-apply settings for the new instance
		this->setVolume(this->_volume);
		this->setTempo(this->_tempo);
		this->setLooping(this->_looping);
		this->seek(this->_seek);

		if (this->is3D()) {
			this->setPosition(this->_location);
			this->set3D(this->_maxDistance, this->_minDistance);
		}

		BASS_ChannelPlay(this->_channel, static_cast<BOOL>(this->_stream || this->_looping)); // actualy play the thing
		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::stop() {
		if (!this->isCreated()) return;
		BASS_ChannelStop(this->_channel);
		if (!this->_stream) {
			BASS_ChannelFree(this->_channel);
		} else {
			BASS_StreamFree(this->_channel);
		}

		rawrbox::BASSUtils::checkBASSError();
		this->_channel = 0;
	}

	void SoundInstance::pause() {
		if (!this->isCreated()) return;

		BASS_ChannelPause(this->_channel);
		rawrbox::BASSUtils::checkBASSError();
	}

	// UTILS ----
	uint32_t SoundInstance::id() const {
		return this->_channel;
	}

	bool SoundInstance::isValid() const {
		return this->_sample != 0;
	}

	bool SoundInstance::isCreated() const {
		BASS_CHANNELINFO info;
		return this->_channel != 0 && (BASS_ChannelGetInfo(this->_channel, &info) != 0);
	}

	bool SoundInstance::is3D() const {
		return (this->_flags & SoundFlags::SOUND_3D) > 0;
	}

	float SoundInstance::getTempo() const {
		return this->_tempo;
	}

	float SoundInstance::getVolume() const {
		return this->_volume;
	}

	double SoundInstance::getSeek() const {
		if (!this->isCreated()) return 0.;

		QWORD pos = BASS_ChannelGetPosition(this->_channel, BASS_POS_BYTE);
		return pos != -1. ? BASS_ChannelBytes2Seconds(this->_channel, pos) : 0.;
	}

	bool SoundInstance::isLooping() const {
		return this->_looping;
	}

	bool SoundInstance::isPlaying() const {
		return BASS_ChannelIsActive(this->_channel) == BASS_ACTIVE_PLAYING;
	}

	bool SoundInstance::isPaused() const {
		return BASS_ChannelIsActive(this->_channel) == BASS_ACTIVE_PAUSED;
	}

	bool SoundInstance::isHTTPStream() const {
		return BASS_ChannelGetTags(this->_channel, BASS_TAG_HTTP) == nullptr;
	}

	std::vector<float> SoundInstance::getFFT(int bass_length) const {
		std::vector<float> buffer;
		if (!this->isCreated()) return buffer;

		auto flag = BASS_DATA_FFT256;
		switch (bass_length) {
			case 256:
				flag = BASS_DATA_FFT256;
				break;
			case 512:
				flag = BASS_DATA_FFT512;
				break;
			case 1024:
				flag = BASS_DATA_FFT1024;
				break;
			case 2048:
				flag = BASS_DATA_FFT2048;
				break;
			case 4096:
				flag = BASS_DATA_FFT4096;
				break;
			default:
				throw this->_logger->error("Unknown FFT length {}, should be power of 2! Check: http://bass.radio42.com/help/html/a13cfef0-1056-bb94-81c4-a4fdf21bd463.htm", bass_length);
		}

		buffer.resize(bass_length);
		BASS_ChannelGetData(this->_channel, &buffer.front(), flag);
		rawrbox::BASSUtils::checkBASSError();

		return buffer;
	}

	const rawrbox::Vector3f& SoundInstance::getPosition() const {
		return this->_location;
	}
	// --------------

	void SoundInstance::setBeatSettings(float bandwidth, float center_freq, float release_time) {
		if ((this->_flags & SoundFlags::BEAT_DETECTION) == 0) throw this->_logger->error("Load flag BEAT_DETECTION not set!");
		if (!this->isCreated()) return;

		BASS_FX_BPM_BeatSetParameters(this->_channel, bandwidth, center_freq, release_time);
		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::setVolume(float volume) {
		this->_volume = volume;
		if (!this->isCreated()) return;

		BASS_ChannelSetAttribute(this->_channel, BASS_ATTRIB_VOL, volume);
		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::setTempo(float tempo) {
		this->_tempo = std::clamp(tempo, 0.01F, 2.F);
		if (!this->isCreated()) return;

		float m_fDefaultFrequency = 0.F;
		BASS_ChannelGetAttribute(this->_channel, BASS_ATTRIB_FREQ, &m_fDefaultFrequency);
		BASS_ChannelSetAttribute(this->_channel, BASS_ATTRIB_FREQ, this->_tempo * m_fDefaultFrequency);

		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::setLooping(bool loop) {
		this->_looping = loop;
		if (!this->isCreated()) return;

		if (this->_looping) {
			BASS_ChannelFlags(this->_channel, BASS_MUSIC_LOOP, BASS_MUSIC_LOOP);
		} else {
			BASS_ChannelFlags(this->_channel, 0, 0xff);
		}

		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::seek(double seek) {
		this->_seek = seek;
		if (!this->isCreated() || this->isHTTPStream()) return;

		QWORD bytePosition = BASS_ChannelSeconds2Bytes(this->_channel, seek);
		QWORD byteLength = BASS_ChannelGetLength(this->_channel, BASS_POS_BYTE);
		BASS_ChannelSetPosition(this->_channel, std::clamp<QWORD>(bytePosition, 0, byteLength - 1), BASS_POS_BYTE);

		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::setPosition(const rawrbox::Vector3f& pos) {
		this->_location = pos;
		if (!this->isCreated() || !this->is3D()) return;

		BASS_3DVECTOR location = {pos.x, pos.y, pos.z};
		BASS_ChannelSet3DPosition(this->_channel, &location, nullptr, nullptr);
		BASS_Apply3D();

		rawrbox::BASSUtils::checkBASSError();
	}

	void SoundInstance::set3D(float maxDistance, float minDistance) {
		this->_maxDistance = maxDistance;
		this->_minDistance = minDistance;
		if (!this->isCreated() || !this->is3D()) return;

		BASS_ChannelSet3DAttributes(this->_channel, -1, this->_minDistance * 1000.F, this->_maxDistance * 1000.F, -1, -1, -1);
		BASS_Apply3D();

		rawrbox::BASSUtils::checkBASSError();
	}
} // namespace rawrbox
