
#include <rawrbox/engine/static.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/utils/time.hpp>

#include <filesystem>
#include <utility>

namespace rawrbox {
	Mod::Mod(std::string id, std::string folderName) : _folder(std::move(folderName)), _id(std::move(id)) {}

	void Mod::init(Scripting& scripting_) {
		this->_scripting = &scripting_;

		auto& lua = this->_scripting->getLua();
		this->_environment = {lua, sol::create, lua.globals()};
	}

	bool Mod::load() {
		auto& lua = this->_scripting->getLua();

		this->_modTable = lua.create_table();
		this->_environment["MOD"] = this->_modTable;

		// Default utils -----
		this->_environment["curtime"] = []() { return rawrbox::TimeUtils::curtime(); };
		this->_environment["time"] = []() { return rawrbox::TimeUtils::time(); };
		this->_environment["DELTA_TIME"] = []() { return rawrbox::DELTA_TIME; };
		this->_environment["FIXED_DELTA_TIME"] = []() { return rawrbox::FIXED_DELTA_TIME; };
		// --------------

		this->_scripting->addGlobals(*this);
		this->_scripting->loadLuaExtensions(this->_environment, lua);

		// Setup entry points -----
		std::string entrypoint = "init.lua";

		// Load init script (SHARED)
		if (std::filesystem::exists(this->_folder + "/" + entrypoint)) {
			this->_scripting->setIncludePath(this->_folder + "/");
			this->_scripting->loadLuaFile(this->_folder + "/" + entrypoint, this->_environment, lua);
		} else {
			return false;
		}
		// -----

		this->preLoad();
		return true;
	}

	void Mod::preLoad() {}

	// UTILS ----
	const std::string& Mod::getID() const { return this->_id; }
	const std::string& Mod::getFolder() const { return this->_folder; }

	const rawrbox::Scripting& Mod::getScripting() const { return *this->_scripting; }
	sol::environment& Mod::getEnvironment() { return this->_environment; }
	// -----
} // namespace rawrbox
