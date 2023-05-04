#pragma once

#include <rawrBox/bass/sound_flags.hpp>
#include <rawrBox/math/vector2.hpp>
#include <rawrBox/utils/event.hpp>

#include <cmath>
#include <vector>

namespace rawrBox {
	class SoundInstance {

	protected:
		uint32_t _sample = 0;
		uint32_t _channel = 0;
		uint32_t _flags = rawrBox::SoundFlags::NONE;

		rawrBox::Vector2f _location = {std::nanf(""), std::nanf("")};
		float _volume = 1.F;
		float _pitch = 1.F;
		float _maxDistance = 0.F;

		bool _looping = false;
		bool _stream = false;

	protected:
		void checkError();
		uint32_t getNextAvailableChannel();

	public:
		rawrBox::Event<double> onBEAT;
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

		[[nodiscard]] uint32_t id() const;
		[[nodiscard]] bool isValid() const;
		[[nodiscard]] bool isCreated() const;
		[[nodiscard]] bool is3D() const;

		[[nodiscard]] float getPitch() const;
		[[nodiscard]] float getVolume() const;

		[[nodiscard]] bool getLooping() const;
		[[nodiscard]] const rawrBox::Vector2f& getPosition() const;

		[[nodiscard]] const std::vector<float> getFFT(int bass_length);

		void setBeatSettings(float bandwidth, float center_freq, float release_time);

		bool isPlaying();
		bool isHTTP();

		void setVolume(float volume);
		void setPitch(float pitch);
		void setLooping(bool loop);
		void setPosition(const rawrBox::Vector2f& location);
		void set3D(float maxDistance);
	};
} // namespace rawrBox
