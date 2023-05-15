#pragma once

#include <rawrbox/bass/sound/base.hpp>
#include <rawrbox/utils/event_named.hpp>

#include <filesystem>
#include <unordered_map>

namespace rawrbox {
	class BASSManager {
	protected:
		bool _initialized = false;

		float _masterVolume = 1.0F;
		bool _muteOnUnfocus = true;

		rawrbox::Vector3f _oldLocation = {};

	public:
		std::unordered_map<std::string, std::shared_ptr<rawrbox::SoundBase>> sounds = {}; // Keep alive the sounds

		rawrbox::EventNamed<std::pair<uint32_t, double>> onBEAT;
		rawrbox::EventNamed<std::pair<uint32_t, float>> onBPM;
		rawrbox::EventNamed<uint32_t> onSoundEnd;

		void initialize();
		void shutdown();

		std::shared_ptr<rawrbox::SoundBase> loadSound(const std::filesystem::path& path, uint32_t flags = SoundFlags::NONE);
		std::shared_ptr<rawrbox::SoundBase> loadHTTPSound(const std::string& url, uint32_t flags = SoundFlags::NONE);

		// UTILS -----
		[[nodiscard]] float getMasterVolume() const;

		void setMasterVolume(float volume, bool set = true);
		void setMuteOnUnfocus(bool set);
		void setHasFocus(bool hasFocus);
		void setListenerLocation(const rawrbox::Vector3f& location, const rawrbox::Vector3f& front = {0, 0, -1}, const rawrbox::Vector3f& top = {0, -1, 0});
		// -----
	};
} // namespace rawrbox
