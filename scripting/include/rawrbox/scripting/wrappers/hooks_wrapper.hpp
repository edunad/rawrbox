#pragma once

#include <sol/sol.hpp>

namespace rawrbox {
	class Hooks;

	class HooksWrapper {
	protected:
		rawrbox::Hooks* _hooks = nullptr;

	public:
		HooksWrapper(rawrbox::Hooks* hooks_);
		HooksWrapper(const HooksWrapper&) = default;
		HooksWrapper(HooksWrapper&&) = default;
		HooksWrapper& operator=(const HooksWrapper&) = default;
		HooksWrapper& operator=(HooksWrapper&&) = default;
		virtual ~HooksWrapper() = default;

		virtual void listen(const std::string& hook, const std::string& name, sol::function callback);
		virtual void call(const std::string& hook, sol::variadic_args args);
		virtual void remove(const std::string& hook, const std::string& id);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
