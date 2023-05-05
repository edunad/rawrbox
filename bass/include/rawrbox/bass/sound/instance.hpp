#pragma once

#include <rawrBox/bass/sound/flags.hpp>
#include <rawrBox/math/vector3.hpp>
#include <rawrBox/utils/event.hpp>

#include <cmath>
#include <vector>

namespace rawrBox {
	class SoundInstance {

	protected:
		uint32_t _sample = 0;
		uint32_t _channel = 0;
		uint32_t _flags = rawrBox::SoundFlags::NONE;

		// 3D SUPPORT ----
		rawrBox::Vector3f _location = {std::nanf(""), std::nanf(""), std::nanf("")};
		float _maxDistance = 0.F;
		float _minDistance = 0.F;
		// -------------

		float _volume = 1.F;
		float _tempo = 1.F;
		double _seek = 0.F;

		bool _looping = false;
		bool _stream = false;

	protected:
		[[nodiscard]] uint32_t getNextAvailableChannel() const;

	public:
		rawrBox::Event<double> onBEAT;
		rawrBox::Event<float> onBPM;
		rawrBox::Event<> onEnd;

		SoundInstance() = default;
		SoundInstance(uint32_t audioSample, bool isStream, uint32_t flags);

		~SoundInstance();
		SoundInstance(SoundInstance&&) = delete;
		SoundInstance& operator=(SoundInstance&&) = delete;
		SoundInstance(const SoundInstance&) = delete;
		SoundInstance& operator=(const SoundInstance&) = delete;

		void play();
		void stop();

		// UTILS ----
		[[nodiscard]] uint32_t id() const;
		[[nodiscard]] bool isValid() const;
		[[nodiscard]] bool isCreated() const;
		[[nodiscard]] bool is3D() const;

		[[nodiscard]] float getTempo() const;
		[[nodiscard]] float getVolume() const;
		[[nodiscard]] double getSeek() const;

		[[nodiscard]] bool isLooping() const;
		[[nodiscard]] bool isPlaying() const;
		[[nodiscard]] bool isPaused() const;
		[[nodiscard]] bool isHTTPStream() const;

		[[nodiscard]] const std::vector<float> getFFT(int bass_length) const;

		[[nodiscard]] const rawrBox::Vector3f& getPosition() const;
		// ------------------

		void setBeatSettings(float bandwidth, float center_freq, float release_time);
		void setVolume(float volume);
		void setTempo(float tempo);
		void seek(double seek);
		void setLooping(bool loop);
		void setPosition(const rawrBox::Vector3f& location);
		void set3D(float maxDistance, float minDistance = 0.F);
	};
} // namespace rawrBox
