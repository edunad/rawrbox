#pragma once

#include <rawrbox/network/scripting/wrappers/http_wrapper.hpp>
#include <rawrbox/network/scripting/wrappers/packet_wrapper.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class NetworkPlugin : public rawrbox::ScriptingPlugin {
	public:
		void registerTypes(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::HTTPWrapper::registerLua(L);
			rawrbox::PacketWrapper::registerLua(L);
		};

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::HTTP", "./lua/http_enums.lua");
		}
	};
} // namespace rawrbox
