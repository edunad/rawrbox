
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/scripting/utils/lua.hpp>
#include <rawrbox/utils/path.hpp>

#include <Luau/Compiler.h>

namespace rawrbox {
	Mod::Mod(lua_State* L, std::string id, std::filesystem::path folderPath) : _L(L), _folder(std::move(folderPath)), _id(std::move(id)), _LSandbox(lua_newthread(L)), _modTable(_LSandbox) {
		if (this->_LSandbox == nullptr) throw _logger->error("Invalid lua handle");
		luaL_sandboxthread(this->_LSandbox);
	}

	Mod::~Mod() {
		this->_L = nullptr;
		this->_LSandbox = nullptr;
	}

	void Mod::init() {
		// Initialize table ---
		this->_modTable = luabridge::newTable(this->_LSandbox);
		luabridge::setGlobal(this->_LSandbox, this->_modTable, "MOD");
		// --------------------
	}

	void Mod::load() {
		if (this->_LSandbox == nullptr) throw _logger->error("Invalid lua sandbox environment");

		try {
			rawrbox::LuaUtils::compileAndLoad(this->_LSandbox, this->getID(), this->getEntryFilePath());
		} catch (std::exception& error) {
			throw this->_logger->error("{}", error.what());
		}

		if (lua_resume(this->_LSandbox, this->_L, 0) != 0) {
			throw this->_logger->error("{}", rawrbox::LuaUtils::getError(this->_LSandbox));
		}
	}

	// UTILS ----
	const std::string& Mod::getID() const { return this->_id; }
	const std::string Mod::getEntryFilePath() const { return fmt::format("{}/init.luau", this->_folder.generic_string()); }
	const std::filesystem::path& Mod::getFolder() const { return this->_folder; }

	lua_State* Mod::getEnvironment() { return this->_LSandbox; }
	// -----
} // namespace rawrbox
