#pragma once

#include <rawrbox/bass/manager.hpp>
#include <rawrbox/bass/scripting/wrapper/instance_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class BASSWrapper {

	public:
		BASSWrapper() = default;
		BASSWrapper(const BASSWrapper&) = default;
		BASSWrapper(BASSWrapper&&) = default;
		BASSWrapper& operator=(const BASSWrapper&) = default;
		BASSWrapper& operator=(BASSWrapper&&) = default;
		virtual ~BASSWrapper() = default;

		// UTILS -----
		virtual float getMasterVolume();
		virtual void setMasterVolume(float volume, sol::optional<bool> set);
		virtual void setListenerLocation(const rawrbox::Vector3f& location, sol::optional<rawrbox::Vector3f> front, sol::optional<rawrbox::Vector3f> top);
		// -----

		// LOAD -----
		virtual rawrbox::SoundInstanceWrapper loadSound(const std::string& path, sol::optional<uint32_t> flags, sol::this_environment modEnv);
		virtual rawrbox::SoundInstanceWrapper loadHTTPSound(const std::string& url, sol::optional<uint32_t> flags);
		// -----

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
