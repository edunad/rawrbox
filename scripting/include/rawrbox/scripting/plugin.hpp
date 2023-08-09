#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class Mod;

	class Plugin {
	public:
		Plugin() = default;
		Plugin(const Plugin &) = default;
		Plugin(Plugin &&) = default;
		Plugin &operator=(const Plugin &) = default;
		Plugin &operator=(Plugin &&) = default;
		virtual ~Plugin() = default;

		virtual void registerTypes(sol::state & /*_lua*/) {}
		virtual void registerGlobal(sol::environment & /*_env*/) {}
	};
} // namespace rawrbox
