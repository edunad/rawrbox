#pragma once

#include <rawrbox/scripting/mod.hpp>

#include <sol/sol.hpp>

#include <string>
#include <unordered_map>
#include <vector>

#include "rawrbox/utils/event.hpp"

namespace rawrbox {
	class Scripting {
	protected:
		std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> _mods = {};
		std::vector<std::string> _loadedLuaFiles = {};

		sol::state _lua = {};
		std::string _includePathPrefix = "";

		// LOAD ----
		virtual void loadLibraries();
		virtual void loadTypes();
		// ----

	public:
		Scripting() = default;
		Scripting(const Scripting&) = delete;
		Scripting(Scripting&&) = delete;
		Scripting& operator=(const Scripting&) = delete;
		Scripting& operator=(Scripting&&) = delete;
		virtual ~Scripting() = default;

		rawrbox::Event<Mod&> onRegisterGlobals;
		rawrbox::Event<> onRegisterTypes;

		virtual void init();
		virtual void addGlobals(Mod& mod);

		virtual void update();
		virtual void fixedUpdate();

		// LOAD -----
		virtual bool loadLuaFile(const std::string& path, const sol::environment& env, sol::state& lua, bool setIncludeDirectory = true);
		virtual void loadLuaExtensions(sol::environment& env, sol::state& lua);
		[[nodiscard]] virtual bool isLuaFileMounted(const std::string& path) const;
		//------

		[[nodiscard]] virtual sol::state& getLua();
		[[nodiscard]] virtual const std::unordered_map<std::string, std::unique_ptr<Mod>>& getMods() const;
		[[nodiscard]] virtual const std::vector<std::string> getModsIds() const;

		void setIncludePath(const std::string& path);
		[[nodiscard]] const std::string& getIncludePath() const;

		template <typename... CallbackArgs>
		void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : this->_mods) {
				mod.second->call(hookName, args...);
			}
		}
	};
} // namespace rawrbox
