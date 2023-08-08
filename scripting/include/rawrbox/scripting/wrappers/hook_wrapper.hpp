#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class Hooks;

	class HookWrapper {
	protected:
		rawrbox::Hooks* _hooks = nullptr;

	public:
		HookWrapper(rawrbox::Hooks* hooks_);
		HookWrapper(const HookWrapper&) = default;
		HookWrapper(HookWrapper&&) = default;
		HookWrapper& operator=(const HookWrapper&) = default;
		HookWrapper& operator=(HookWrapper&&) = default;
		virtual ~HookWrapper() = default;

		virtual void listen(const std::string& hook, const std::string& name, sol::function callback);
		virtual void call(const std::string& hook, sol::variadic_args args);
		virtual void remove(const std::string& hook, const std::string& id);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
