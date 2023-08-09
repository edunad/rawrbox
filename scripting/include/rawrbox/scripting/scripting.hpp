#pragma once

#include <rawrbox/scripting/hooks.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/utils/event.hpp>
#include <rawrbox/utils/file_watcher.hpp>
#include <rawrbox/utils/string.hpp>

#include <sol/sol.hpp>

#include <string>
#include <typeinfo>
#include <unordered_map>
#include <vector>

namespace rawrbox {

	template <typename T>
	concept isLuaType = requires(sol::state lua) {
		{ T::registerLua(lua) };
	};

	class Scripting {
	protected:
		std::unordered_map<std::string, std::unique_ptr<rawrbox::Mod>> _mods = {};
		std::unordered_map<std::string, std::vector<std::string>> _loadedLuaFiles = {};

		std::unique_ptr<rawrbox::FileWatcher> _watcher = nullptr;
		std::unique_ptr<rawrbox::Hooks> _hooks = nullptr;
		std::unique_ptr<sol::state> _lua = nullptr;

		std::vector<std::unique_ptr<rawrbox::Plugin>> _plugins = {};

		bool _hotReloadEnabled = false;

		// LOAD ----
		virtual void loadLibraries();
		virtual void loadTypes();
		virtual void loadLuaExtensions(rawrbox::Mod* mod);
		virtual void loadGlobals(rawrbox::Mod* mod);
		// ----

		virtual void registerLoadedFile(const std::string& modId, const std::string& filePath);
		virtual void hotReload(const std::string& filePath);

	public:
		Scripting(int hotReloadMs = 0);
		Scripting(const Scripting&) = delete;
		Scripting(Scripting&&) = delete;
		Scripting& operator=(const Scripting&) = delete;
		Scripting& operator=(Scripting&&) = delete;
		virtual ~Scripting();

		// EVENTS ----
		rawrbox::Event<> onRegisterTypes;
		rawrbox::Event<rawrbox::Mod*> onRegisterGlobals;
		rawrbox::Event<rawrbox::Mod*> onLoadExtensions;
		rawrbox::Event<rawrbox::Mod*> onModHotReload;
		// -------

		// PLUGINS ---
		template <typename T = rawrbox::Plugin, typename... CallbackArgs>
		void registerPlugin(CallbackArgs&&... args) {
			auto plugin = std::make_unique<T>(std::forward<CallbackArgs>(args)...);

			fmt::print("[RawrBox-Scripting] Registered lua plugin '{}'\n", rawrbox::StrUtils::replace(typeid(T).name(), "class rawrbox::", ""));
			this->_plugins.push_back(std::move(plugin));
		}
		// -----

		virtual void init();

		// LOAD -----
		virtual void load();
		virtual bool loadLuaFile(const std::string& path, const sol::environment& env, sol::state& lua);
		[[nodiscard]] virtual bool isLuaFileMounted(const std::string& path) const;
		//------

		// UTILS -----
		[[nodiscard]] virtual sol::state& getLua();
		[[nodiscard]] virtual const std::unordered_map<std::string, std::unique_ptr<Mod>>& getMods() const;
		[[nodiscard]] virtual const std::vector<std::string> getModsIds() const;

		[[nodiscard]] bool hotReloadEnabled() const;

		template <typename T>
		void registerType() {
			if (this->_lua == nullptr) throw std::runtime_error("[RawrBox-Scripting] LUA is not set! Reference got destroyed?");

			if constexpr (isLuaType<T>) {
				T::registerLua(*this->_lua);
			} else {
				throw std::runtime_error("[RawrBox-Scripting] Type missing 'registerLua'");
			}
		}

		template <typename... CallbackArgs>
		void call(const std::string& hookName, CallbackArgs&&... args) {
			for (auto& mod : this->_mods) {
				mod.second->call(hookName, args...);
			}
		}
		//------
	};
} // namespace rawrbox
