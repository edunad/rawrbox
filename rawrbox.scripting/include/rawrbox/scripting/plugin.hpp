#pragma once

#include <filesystem>

struct lua_State;
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

		virtual void modifyMod(rawrbox::Mod& /*mod*/) {};

		virtual void registerTypes(lua_State* /*l*/) {};
		virtual void registerGlobal(lua_State* /*l*/) {};

		virtual void loadLibraries(lua_State* /*l*/) {};
		virtual bool loadFile(rawrbox::Mod& /*mod*/, lua_State* /*l*/, const std::filesystem::path& /*path*/) { return false; }
	};
} // namespace rawrbox