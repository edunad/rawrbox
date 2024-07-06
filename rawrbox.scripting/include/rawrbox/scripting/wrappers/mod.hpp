#pragma once

#include <rawrbox/scripting/mod.hpp>

namespace rawrbox {
	class MODWrapper {
	protected:
		rawrbox::Mod* _mod = nullptr;

	public:
		MODWrapper(rawrbox::Mod* mod);
		MODWrapper(const MODWrapper&) = default;
		MODWrapper(MODWrapper&&) = default;
		MODWrapper& operator=(const MODWrapper&) = default;
		MODWrapper& operator=(MODWrapper&&) = default;
		virtual ~MODWrapper() = default;

		virtual void call(const std::string& method, const luabridge::LuaRef& ref) const;

		[[nodiscard]] virtual std::string getID() const;
		[[nodiscard]] virtual std::string getFolder() const;
		[[nodiscard]] virtual std::string getEntryFilePath() const;
		[[nodiscard]] virtual luabridge::LuaRef getMetadata(lua_State* L) const;

		static void registerLua(lua_State* L);
	};
} // namespace rawrbox
