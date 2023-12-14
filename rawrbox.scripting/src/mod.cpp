
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/scripting.hpp>

#include <filesystem>
#include <utility>

namespace rawrbox {
	Mod::Mod(std::string id, std::filesystem::path folderPath) : _folder(std::move(folderPath)), _id(std::move(id)) {}
	Mod::~Mod() {
		this->_environment.reset();
	}

	void Mod::init() {
		auto& lua = rawrbox::SCRIPTING::getLUA();
		this->_environment = {lua, sol::create, lua.globals()};
	}

	bool Mod::load() {
		auto& lua = rawrbox::SCRIPTING::getLUA();

		this->_modTable = lua.create_table();
		this->_environment["MOD"] = this->_modTable;

		// Load init script
		auto pth = this->getEntryFilePath();
		if (std::filesystem::exists(pth)) {
			if (!rawrbox::SCRIPTING::loadLuaFile(pth, this->_environment)) {
				return false;
			}
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
	const std::string Mod::getEntryFilePath() const { return fmt::format("{}/init.lua", this->_folder.generic_string()); }
	const std::filesystem::path& Mod::getFolder() const { return this->_folder; }

	sol::environment& Mod::getEnvironment() { return this->_environment; }
	// -----
} // namespace rawrbox
