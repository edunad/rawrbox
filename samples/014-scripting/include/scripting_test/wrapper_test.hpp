#pragma once

#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/scripting/utils/lua.hpp>

namespace rawrbox {
	class TestWrapper {
	public:
		inline static void registerLua(lua_State* L) {
			luabridge::getGlobalNamespace(L)
			    .beginNamespace("test")
			    .addFunction("hello", [](const std::string& str) {
				    return "hello " + str + " mew mew";
			    })
			    .endNamespace();
		}
	};

	class TestPlugin : public rawrbox::ScriptingPlugin {
		void registerTypes(lua_State* L) override {
			TestWrapper::registerLua(L);
		}
	};
} // namespace rawrbox
