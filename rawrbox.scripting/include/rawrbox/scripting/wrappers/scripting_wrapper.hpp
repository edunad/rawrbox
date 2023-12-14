#pragma once

#include <rawrbox/scripting/scripting.hpp>

namespace rawrbox {
	class ScriptingWrapper {
	public:
		ScriptingWrapper() = default;
		ScriptingWrapper(const ScriptingWrapper&) = default;
		ScriptingWrapper(ScriptingWrapper&&) = default;
		ScriptingWrapper& operator=(const ScriptingWrapper&) = default;
		ScriptingWrapper& operator=(ScriptingWrapper&&) = default;
		virtual ~ScriptingWrapper() = default;

		virtual sol::object getMod(const std::string& id, sol::this_state lua);
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
