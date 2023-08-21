#pragma once

#include <rawrbox/bass/scripting/wrapper/instance_wrapper.hpp>
#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class SoundLoaderWrapper {
		rawrbox::Mod* _mod;

	public:
		SoundLoaderWrapper(rawrbox::Mod* mod_);
		SoundLoaderWrapper(const SoundLoaderWrapper&) = default;
		SoundLoaderWrapper(SoundLoaderWrapper&&) = default;
		SoundLoaderWrapper& operator=(const SoundLoaderWrapper&) = default;
		SoundLoaderWrapper& operator=(SoundLoaderWrapper&&) = default;
		virtual ~SoundLoaderWrapper() = default;

		virtual rawrbox::SoundInstanceWrapper get(const std::string& path, sol::optional<uint32_t> flags);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
