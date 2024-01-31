
#include <rawrbox/scripting/wrappers/mod.hpp>

namespace rawrbox {
	MODWrapper::MODWrapper(rawrbox::Mod* mod_) : _mod(mod_) {}

	// Can be a bit expensive to call this, since it will create a new table on the new env every time
	void MODWrapper::call(const std::string& method, const luabridge::LuaRef& ref) const {
		if (this->_mod == nullptr) throw std::runtime_error("Invalid mod reference");

		auto modEnv = this->_mod->getEnvironment();
		auto func = luabridge::getGlobal(modEnv, method.c_str());
		if (!func.isValid()) throw std::runtime_error(fmt::format("Failed to find method {}", method));
		if (!func.isCallable()) throw std::runtime_error("Invalid method id, not callable");

		auto argTable = luabridge::newTable(modEnv);
		rawrbox::LuaUtils::getVariadicArgs(ref, argTable);

		luabridge::call(func, argTable);
	}

	std::string MODWrapper::getID() const {
		if (this->_mod == nullptr) throw std::runtime_error("Invalid mod reference");
		return this->_mod->getID();
	}

	std::string MODWrapper::getFolder() const {
		if (this->_mod == nullptr) throw std::runtime_error("Invalid mod reference");
		return this->_mod->getFolder().generic_string();
	}

	std::string MODWrapper::getEntryFilePath() const {
		if (this->_mod == nullptr) throw std::runtime_error("Invalid mod reference");
		return this->_mod->getEntryFilePath();
	}

	void MODWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginClass<rawrbox::MODWrapper>("mod")
		    .addFunction("call", &MODWrapper::call)
		    .addFunction("getID", &MODWrapper::getID)
		    .addFunction("getFolder", &MODWrapper::getFolder)
		    .addFunction("getEntryFilePath", &MODWrapper::getEntryFilePath)
		    .endClass();
	}
} // namespace rawrbox
