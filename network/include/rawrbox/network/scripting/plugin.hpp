#pragma once

#include <rawrbox/network/scripting/wrappers/http_wrapper.hpp>
#include <rawrbox/scripting/scripting.hpp>

namespace rawrbox {
	class NetworkPlugin : public rawrbox::Plugin {
	public:
		void registerTypes(sol::state& lua) override {
			rawrbox::HTTPWrapper::registerLua(lua);
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-NetworkPlugin] Tried to register plugin on invalid mod!");

			auto& env = mod->getEnvironment();
			env["http"] = rawrbox::HTTPWrapper();
		}

		void loadLuaExtensions(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-NetworkPlugin] Tried to register plugin on invalid mod!");
			rawrbox::SCRIPTING::loadLuaFile("./lua/http.lua", mod->getEnvironment());
		}
	};
} // namespace rawrbox
