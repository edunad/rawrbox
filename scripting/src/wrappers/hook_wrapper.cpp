#include <rawrbox/scripting/hook.hpp>
#include <rawrbox/scripting/wrappers/hook_wrapper.hpp>

namespace rawrbox {
	HookWrapper::HookWrapper(rawrbox::Hooks* hooks_) : _hooks(hooks_) {}

	void HookWrapper::listen(const std::string& hook, const std::string& name, sol::function callback) {
		this->_hooks->listen(hook, name, callback);
	}

	void HookWrapper::call(const std::string& hook, sol::variadic_args args) {
		this->_hooks->call(hook, args);
	}

	void HookWrapper::remove(const std::string& hook, const std::string& id) {
		this->_hooks->remove(hook, id);
	}

	void HookWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<HookWrapper>("hooks",
		    sol::no_constructor,
		    "add", &HookWrapper::listen,
		    "call", &HookWrapper::call,
		    "remove", &HookWrapper::remove);
	}
} // namespace rawrbox
