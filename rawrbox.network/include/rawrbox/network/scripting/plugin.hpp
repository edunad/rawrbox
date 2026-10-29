#pragma once

#include <rawrbox/network/scripting/global/http.hpp>
#include <rawrbox/network/scripting/wrappers/packet.hpp>
#include <rawrbox/scripting/manager.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class NetworkScripting : public rawrbox::ScriptingPlugin {
	public:
		NetworkScripting() { rawrbox::SCRIPTING::onModUnload += &rawrbox::HTTPGlobal::clear; }
		NetworkScripting(const NetworkScripting&) = delete;
		NetworkScripting(NetworkScripting&&) = delete;
		NetworkScripting& operator=(const NetworkScripting&) = delete;
		NetworkScripting& operator=(NetworkScripting&&) = delete;
		~NetworkScripting() override { rawrbox::SCRIPTING::onModUnload -= &rawrbox::HTTPGlobal::clear; }

		void registerTypes(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::PacketWrapper::registerLua(L);
		};

		void registerGlobal(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::HTTPGlobal::registerLua(L);
		};

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::HTTP", "./lua/enums/http.lua");
		}
	};
} // namespace rawrbox
