#pragma once

#include <rawrbox/bass/scripting/wrappers/bass.hpp>
#include <rawrbox/bass/scripting/wrappers/instance.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class BASScripting : public rawrbox::ScriptingPlugin {
	public:
		void registerTypes(lua_State* L) override {
			rawrbox::SoundInstanceWrapper::registerLua(L);
			rawrbox::BASSWrapper::registerLua(L);
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::BASS", "./lua/bass_enums.lua");
		}
	};
} // namespace rawrbox
