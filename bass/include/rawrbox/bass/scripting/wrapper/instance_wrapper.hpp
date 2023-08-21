#pragma once

#include <rawrbox/bass/sound/instance.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class SoundInstanceWrapper {
	protected:
		std::shared_ptr<rawrbox::SoundInstance> _ref; // Sound wrapper needs someone to own it

	public:
		SoundInstanceWrapper(const std::shared_ptr<rawrbox::SoundInstance>& instance);
		SoundInstanceWrapper(const SoundInstanceWrapper&) = default;
		SoundInstanceWrapper(SoundInstanceWrapper&&) = delete;
		SoundInstanceWrapper& operator=(const SoundInstanceWrapper&) = default;
		SoundInstanceWrapper& operator=(SoundInstanceWrapper&&) = delete;
		virtual ~SoundInstanceWrapper();

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

		[[nodiscard]] virtual const sol::table getFFT(int bass_length, sol::state& lua) const;

		[[nodiscard]] virtual const rawrbox::Vector3f& getPosition() const;
		// ------------------

		virtual void setBeatSettings(float bandwidth, float center_freq, float release_time);
		virtual void setVolume(float volume);
		virtual void setTempo(float tempo);
		virtual void seek(double seek);
		virtual void setLooping(bool loop);
		virtual void setPosition(const rawrbox::Vector3f& location);
		virtual void set3D(float maxDistance, sol::optional<float> minDistance);

		virtual void onBEAT(sol::function callback);
		virtual void onBPM(sol::function callback);
		virtual void onEnd(sol::function callback);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
