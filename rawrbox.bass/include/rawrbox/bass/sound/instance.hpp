#pragma once

#include <rawrbox/bass/sound/flags.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/logger.hpp>

#include <cmath>
#include <memory>
#include <vector>

namespace rawrbox {
	class SoundInstance {
	protected:
		uint32_t _sample = 0;
		uint32_t _channel = 0;
		uint32_t _flags = rawrbox::SoundFlags::NONE;

		// 3D SUPPORT ----
		rawrbox::Vector3f _location = {std::nanf(""), std::nanf(""), std::nanf("")};
		float _maxDistance = 0.F;
		float _minDistance = 0.F;
		// -------------

		float _volume = 1.F;
		float _tempo = 1.F;
		double _seek = 0.F;

		bool _looping = false;
		bool _stream = false;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-SoundInstance");
		// -------------

		[[nodiscard]] uint32_t getNextAvailableChannel() const;

	public:
		rawrbox::Event<double> onBEAT;
		rawrbox::Event<float> onBPM;
		rawrbox::Event<> onEnd;

		SoundInstance(uint32_t audioSample, bool isStream, uint32_t flags);
		SoundInstance(SoundInstance&&) = delete;
		SoundInstance& operator=(SoundInstance&&) = delete;
		SoundInstance(const SoundInstance&) = delete;
		SoundInstance& operator=(const SoundInstance&) = delete;
		virtual ~SoundInstance();

		virtual void play();
		virtual void pause();
		virtual void stop();

		// UTILS ----
		[[nodiscard]] virtual uint32_t id() const;
		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual bool isCreated() const;
		[[nodiscard]] virtual bool is3D() const;

		[[nodiscard]] virtual float getTempo() const;
		[[nodiscard]] virtual float getVolume() const;
		[[nodiscard]] virtual double getSeek() const;

		[[nodiscard]] virtual bool isLooping() const;
		[[nodiscard]] virtual bool isPlaying() const;
		[[nodiscard]] virtual bool isPaused() const;
		[[nodiscard]] virtual bool isHTTPStream() const;

		[[nodiscard]] virtual std::vector<float> getFFT(int bass_length) const;

		[[nodiscard]] virtual const rawrbox::Vector3f& getPosition() const;
		// ------------------

		virtual void setBeatSettings(float bandwidth, float center_freq, float release_time);
		virtual void setVolume(float volume);
		virtual void setTempo(float tempo);
		virtual void seek(double seek);
		virtual void setLooping(bool loop);
		virtual void setPosition(const rawrbox::Vector3f& pos);
		virtual void set3D(float maxDistance, float minDistance = 0.F);
	};
} // namespace rawrbox
