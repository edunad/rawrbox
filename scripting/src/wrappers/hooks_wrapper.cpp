#include <rawrbox/scripting/hooks.hpp>
#include <rawrbox/scripting/wrappers/hooks_wrapper.hpp>

namespace rawrbox {
	HooksWrapper::HooksWrapper(rawrbox::Hooks* hooks_) : _hooks(hooks_) {}

	void HooksWrapper::listen(const std::string& hook, const std::string& name, sol::function callback) {
		this->_hooks->listen(hook, name, callback);
	}

	void HooksWrapper::call(const std::string& hook, sol::variadic_args args) {
		this->_hooks->call(hook, args);
	}

	void HooksWrapper::remove(const std::string& hook, const std::string& id) {
		this->_hooks->remove(hook, id);
	}

	void HooksWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<HooksWrapper>("hooks",
		    sol::no_constructor,
		    "add", &HooksWrapper::listen,
		    "call", &HooksWrapper::call,
		    "remove", &HooksWrapper::remove);
	}
} // namespace rawrbox
