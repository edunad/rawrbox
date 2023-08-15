#pragma once
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class BASSPlugin : public rawrbox::Plugin {
	public:
		void registerTypes(sol::state& lua) override {
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-BASSPlugin] Tried to register plugin on invalid mod!");
		}

		void loadLuaExtensions(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-BASSPlugin] Tried to register plugin on invalid mod!");
		}
	};
} // namespace rawrbox
