#pragma once

class lua_State;
namespace rawrbox {
	class Mod;

	class ScriptingPlugin {
	public:
		ScriptingPlugin() = default;
		ScriptingPlugin(const ScriptingPlugin&) = default;
		ScriptingPlugin(ScriptingPlugin&&) = default;
		ScriptingPlugin& operator=(const ScriptingPlugin&) = default;
		ScriptingPlugin& operator=(ScriptingPlugin&&) = default;
		virtual ~ScriptingPlugin() = default;

		virtual void registerTypes(lua_State* /*_l*/) = 0;
		virtual void registerGlobal(rawrbox::Mod* /*_mod*/) = 0;
		virtual void loadLuaExtensions(rawrbox::Mod* /*_mod*/) = 0;
	};
} // namespace rawrbox
