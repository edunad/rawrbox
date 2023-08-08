
#include <rawrbox/scripting/wrappers/mod_wrapper.hpp>

namespace rawrbox {
	ModWrapper::ModWrapper(rawrbox::Mod* mod_) : _mod(mod_) {}

	sol::environment ModWrapper::getENV() const {
		if (this->_mod == nullptr) return sol::nil;
		return this->_mod->getEnvironment();
	}

	std::string ModWrapper::getID() const {
		if (this->_mod == nullptr) return "";
		return this->_mod->getID();
	}

	std::string ModWrapper::getFolder() const {
		if (this->_mod == nullptr) return "";
		return this->_mod->getFolder().generic_string();
	}

	std::string ModWrapper::getEntryFilePath() const {
		if (this->_mod == nullptr) return "";
		return this->_mod->getEntryFilePath();
	}

	void ModWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ModWrapper>("Mod",
		    sol::no_constructor,

		    "getENV", &ModWrapper::getENV,
		    "getID", &ModWrapper::getID,
		    "getFolder", &ModWrapper::getFolder,
		    "getEntryFilePath", &ModWrapper::getEntryFilePath);
	}
} // namespace rawrbox
