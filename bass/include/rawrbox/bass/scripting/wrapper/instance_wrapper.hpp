#pragma once

#include <rawrbox/bass/sound/instance.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class SoundInstanceWrapper {
		std::shared_ptr<rawrbox::SoundInstance> ref;

	public:
		SoundInstanceWrapper(const std::shared_ptr<rawrbox::SoundInstance>& instance);

		/*bool getLooping() const;
		void setLooping(bool loop);

		float getVolume() const;
		void setVolume(float val);

		float getPitch() const;
		void setPitch(float val);

		const mainframe::math::Vector2& getPosition() const;
		void setPosition(const mainframe::math::Vector2& location);

		void set3D(float maxDistance);
		void setBeatSettings(float bandwidth, float center_freq, float release_time);

		bool isValid();
		bool isPlaying();
		bool isHTTP();
		uint32_t id();

		sol::table getFFT(int fft_length, sol::this_state lua);

		void stop();
		void play();

		void onBEAT(sol::function callback);
		void onEnd(sol::function callback);*/

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
