#pragma once

#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/utils/event_named.hpp>

#include <filesystem>
#include <unordered_map>

namespace rawrbox {
	class BASS {
	protected:
		static bool _initialized;

		static float _masterVolume;
		static bool _muteOnUnfocus;

		static rawrbox::Vector3f _oldLocation;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------
	public:
		static bool _shutdown;
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::SoundBase>> sounds; // Keep alive the sounds

		static rawrbox::EventNamed<std::pair<uint32_t, double>> onBEAT;
		static rawrbox::EventNamed<std::pair<uint32_t, float>> onBPM;
		static rawrbox::EventNamed<uint32_t> onSoundEnd;

		static void initialize();
		static void shutdown();

		static rawrbox::SoundBase* loadSound(const std::filesystem::path& path, uint32_t flags = SoundFlags::NONE);
		static rawrbox::SoundBase* loadHTTPSound(const std::string& url, uint32_t flags = SoundFlags::NONE);

		// UTILS -----
		static float getMasterVolume();

		static void setMasterVolume(float volume, bool set = true);
		static void setMuteOnUnfocus(bool set);
		static void setHasFocus(bool hasFocus);
		static void setListenerLocation(const rawrbox::Vector3f& location, const rawrbox::Vector3f& front = {0, 0, -1}, const rawrbox::Vector3f& top = {0, -1, 0});
		// -----
	};
} // namespace rawrbox
