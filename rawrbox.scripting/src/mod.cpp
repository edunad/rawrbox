
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/utils/path.hpp>

#include <Luau/Compiler.h>

namespace rawrbox {
	Mod::Mod(std::string id, std::filesystem::path folderPath) : _folder(std::move(folderPath)), _id(std::move(id)), _L(luaL_newstate()), _modTable(_L) {
		if (this->_L == nullptr) throw _logger->error("Invalid lua handle");

		// Inject mod env --
		lua_pushstring(this->_L, this->_folder.generic_string().c_str());
		lua_setglobal(this->_L, "__mod_folder");

		lua_pushstring(this->_L, this->_id.c_str());
		lua_setglobal(this->_L, "__mod_id");
		// -----------------
	}

	Mod::~Mod() {
		this->gc();
		this->_L = nullptr;
	}

	void Mod::init() {
		if (this->_L == nullptr) throw _logger->error("Invalid lua handle");

		// Freeze lua env ---
		// No more modifications to the global table are allowed after this point
		luaL_sandbox(this->_L);
		luaL_sandboxthread(this->_L); // Clone of the _G env that allows modification, but you cannot modify _G directly
		// --------------

		// Initialize mod table, this can be modified ---
		this->_modTable = luabridge::newTable(this->_L);
		luabridge::setGlobal(this->_L, this->_modTable, "MOD");
		//  --------------------
	}

	void Mod::gc() {
		if (this->_L == nullptr) throw _logger->error("Invalid lua handle");
		rawrbox::LuaUtils::collect_garbage(this->_L);
	}

	void Mod::load() {
		if (this->_L == nullptr) throw _logger->error("Invalid lua sandbox environment");
		rawrbox::LuaUtils::compileAndLoad(this->_L, this->getID(), this->getEntryFilePath());
	}

	// UTILS ----
	const std::string& Mod::getID() const { return this->_id; }
	const std::string Mod::getEntryFilePath() const { return fmt::format("{}/init.luau", this->_folder.generic_string()); }
	const std::filesystem::path& Mod::getFolder() const { return this->_folder; }

	lua_State* Mod::getEnvironment() { return this->_L; }
	// -----
} // namespace rawrbox
