#pragma once

#include <nlohmann/json.hpp>
#include <sol/sol.hpp>

#include <fmt/printf.h>

#include <string>

namespace rawrbox {
	class LuaUtils {

	public:
		template <typename... CallbackArgs>
		static sol::protected_function_result runCallback(sol::function func, CallbackArgs&&... args) {
			try {
				auto ret = func(std::forward<CallbackArgs>(args)...);
				if (!ret.valid()) {
					sol::error err = ret;
					fmt::print("[LUA-ERROR] {}\n", err.what());
					return {};
				}

				return ret;
			} catch (const std::exception& err) {
				fmt::print("[LUA-ERROR] {}\n", err.what());
				return {};
			}
		}

		template <typename T>
		static sol::object vectorToLua(const std::vector<T>& vector, sol::state_view& lua) {
			return sol::make_reference<sol::table>(lua, vector);
		}

		template <typename T>
		static std::vector<T> luaToVector(sol::table& table) {
			std::vector<T> arr;
			arr.resize(table.size());

			for (auto& entry : table) {
				if (entry.first.get_type() == sol::type::number) {
					unsigned long index = entry.first.as<unsigned long>() - 1;

					if (entry.second.get_type() == sol::type::nil) continue;
					arr[index] = entry.second.as<T>();
				}
			}

			return arr;
		}

		static sol::object jsonToLuaObject(const nlohmann::json j, sol::state& lua);
		static sol::object jsonToLuaObject(const nlohmann::json j, sol::state_view& lua);
		static nlohmann::json luaToJsonObject(const sol::object& l, bool filterNull = false);

		// @/ == Root content
		// @cats/ == `cats` mod
		// normal_path == current mod
		static std::string getContent(const std::filesystem::path& path, const std::string& modPath = "");
	};
} // namespace rawrbox
