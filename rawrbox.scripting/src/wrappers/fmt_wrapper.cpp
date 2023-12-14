#include <rawrbox/scripting/wrappers/fmt_wrapper.hpp>

#include <fmt/args.h>
#include <fmt/format.h>

namespace rawrbox {
	std::string FMTWrapper::format(const std::string& str, sol::variadic_args va) {
		auto vars = std::vector<sol::object>(va.begin(), va.end());
		fmt::dynamic_format_arg_store<fmt::format_context> args;

		for (auto& var : vars) {
			if (var.get_type() == sol::type::string) {
				args.push_back(var.as<std::string>());
			} else if (var.get_type() == sol::type::number) {
				args.push_back(var.as<double>()); // Lua numbers are always doubles
			}
		}

		return fmt::vformat(str, args);
	}

	void FMTWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<rawrbox::FMTWrapper>("fmt",
		    sol::no_constructor,
		    "format", &rawrbox::FMTWrapper::format);
	}
} // namespace rawrbox
