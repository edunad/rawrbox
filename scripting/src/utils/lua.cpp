
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/string.hpp>

#include <filesystem>

namespace rawrbox {
	sol::object LuaUtils::jsonToLuaObject(const nlohmann::json j, sol::state_view& lua) {
		if (j.is_null()) {
			return sol::nil;
		} else if (j.is_boolean()) {
			return sol::make_object<bool>(lua, j.get<bool>());
		} else if (j.is_number_integer()) {
			return sol::make_object<int>(lua, j.get<int>());
		} else if (j.is_number_unsigned()) {
			return sol::make_object<unsigned int>(lua, j.get<unsigned int>());
		} else if (j.is_number_float()) {
			return sol::make_object<double>(lua, j.get<double>());
		} else if (j.is_string()) {
			return sol::make_object<char*>(lua, j.get<std::string>().c_str());
		} else if (j.is_object()) {
			auto obj = lua.create_table();
			for (nlohmann::json::const_iterator it = j.begin(); it != j.end(); ++it) {
				obj[it.key().c_str()] = jsonToLuaObject(*it, lua);
			}

			return obj.as<sol::object>();
		} else if (j.is_array()) {
			auto obj = lua.create_table();
			unsigned long i = 1;

			for (nlohmann::json::const_iterator it = j.begin(); it != j.end(); ++it) {
				obj[i++] = jsonToLuaObject(*it, lua);
			}

			return obj;
		}

		return sol::nil;
	}

	sol::object LuaUtils::jsonToLuaObject(const nlohmann::json j, sol::state& lua) {
		// NOLINTBEGIN(cppcoreguidelines-slicing)
		sol::state_view view = lua;
		// NOLINTEND(cppcoreguidelines-slicing)
		return jsonToLuaObject(j, view);
	}

	nlohmann::json LuaUtils::luaToJsonObject(const sol::object& l, bool filterNull) {
		switch (l.get_type()) {
			case sol::type::nil: return {};
			case sol::type::boolean: return l.as<bool>();
			case sol::type::number:
				if (l.is<int>()) return l.as<int>();
				if (l.is<double>()) return l.as<double>();
				if (l.is<float>()) return l.as<float>();
			case sol::type::string: return l.as<std::string>();
			case sol::type::table:
				{
					bool isArray = true;
					int index = 0;

					for (auto& entry : l.as<sol::table>()) {
						if (entry.first.get_type() != sol::type::number || entry.first.as<int>() != ++index) {
							isArray = false;
							break;
						}
					}

					if (isArray) {
						auto arr = nlohmann::json::array();
						for (auto& entry : l.as<sol::table>()) {
							auto val = luaToJsonObject(entry.second);
							if (filterNull && val.empty()) continue;

							arr.push_back(val);
						}

						return arr;
					} else {
						auto obj = nlohmann::json::object();
						for (auto& entry : l.as<sol::table>()) {
							auto type = entry.first.get_type();
							if (type != sol::type::number && type != sol::type::string) continue;

							auto val = luaToJsonObject(entry.second);
							if (filterNull && val.empty()) continue;

							if (type == sol::type::number)
								obj[std::to_string(entry.first.as<uint64_t>())] = val;
							else if (type == sol::type::string)
								obj[entry.first.as<std::string>()] = val;
						}

						return obj;
					}
				}
			default:
				return {};
		}
	}

	// @/ == Root content
	// @cats/ == `cats` mod
	// normal_path == current mod
	std::string LuaUtils::getContent(const std::filesystem::path& path, const std::string& modPath) {
		if (path.empty()) return modPath;                               // Invalid path
		if (path.generic_string().starts_with("mods/")) return modPath; // Already has the mod

		auto fixedPath = path.generic_string();
		fixedPath = rawrbox::StrUtils::replace(fixedPath, "./", "");
		fixedPath = rawrbox::StrUtils::replace(fixedPath, "../", "");

		// content/blabalba.png = my current mod
		if (!modPath.empty() && fixedPath.front() != '@') {
			return std::filesystem::path(fmt::format("{}/{}", modPath, fixedPath)).string(); // Becomes mods/mymod/content/blabalba.png
		} else if (fixedPath.front() == '@') {
			auto slashPos = fixedPath.find("/"); // Find the first /
			std::string cleanPath = fixedPath.substr(slashPos + 1);

			// @/textures/blabalba.png = c++ content
			if (fixedPath.rfind("@/", 0) == 0) { // C++
				return std::filesystem::path(fmt::format("content/{}", cleanPath)).string();
			} else { // @otherMod/textures/blabalba.png = @othermod content
				return std::filesystem::path(fmt::format("{}/{}", fixedPath.substr(1, slashPos - 1), cleanPath)).string();
			}
		}

		return fixedPath;
	}

} // namespace rawrbox
