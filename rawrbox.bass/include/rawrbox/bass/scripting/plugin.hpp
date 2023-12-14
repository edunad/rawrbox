#pragma once

#include <rawrbox/bass/scripting/wrapper/bass_wrapper.hpp>
#include <rawrbox/bass/scripting/wrapper/instance_wrapper.hpp>
#include <rawrbox/scripting/scripting.hpp>

namespace rawrbox {
	class BASSPlugin : public rawrbox::Plugin {
	public:
		void registerTypes(sol::state& lua) override {
			rawrbox::SoundInstanceWrapper::registerLua(lua);
			rawrbox::BASSWrapper::registerLua(lua);
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-BASSPlugin] Tried to register plugin on invalid mod!");
			auto& env = mod->getEnvironment();
			env["BASS"] = rawrbox::BASSWrapper();
		}

		void loadLuaExtensions(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-BASSPlugin] Tried to register plugin on invalid mod!");
			rawrbox::SCRIPTING::loadLuaFile("./lua/bass_enums.lua", mod->getEnvironment());
		}
	};
} // namespace rawrbox
