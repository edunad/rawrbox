
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/utils/path.hpp>

#include <Luau/Compiler.h>

namespace rawrbox {
	Mod::Mod(lua_State* L, std::string id, std::filesystem::path folderPath) : _L(L), _folder(std::move(folderPath)), _id(std::move(id)), _LSandbox(lua_newthread(L)), _modTable(_LSandbox) {
		if (this->_LSandbox == nullptr) throw _logger->error("Invalid lua handle");

		// Inject mod env --
		lua_pushstring(this->_LSandbox, this->_folder.generic_string().c_str());
		lua_setglobal(this->_LSandbox, "__mod_folder");

		lua_pushstring(this->_LSandbox, this->_id.c_str());
		lua_setglobal(this->_LSandbox, "__mod_id");
		// -----------------

		// Sandbox thread ----
		luaL_sandboxthread(this->_LSandbox);
		// -------------------
	}

	Mod::~Mod() {
		this->gc();

		this->_L = nullptr;
		this->_LSandbox = nullptr;
	}

	void Mod::init() {
		// Initialize table ---
		this->_modTable = luabridge::newTable(this->_LSandbox);
		luabridge::setGlobal(this->_LSandbox, this->_modTable, "MOD");
		// --------------------
	}

	void Mod::gc() {
		if (this->_LSandbox == nullptr) throw _logger->error("Invalid lua handle");
		rawrbox::LuaUtils::collect_garbage(_LSandbox);
	}

	void Mod::load() {
		if (this->_LSandbox == nullptr) throw _logger->error("Invalid lua sandbox environment");

		try {
			rawrbox::LuaUtils::compileAndLoad(this->_LSandbox, this->getID(), this->getEntryFilePath());
			rawrbox::LuaUtils::run(this->_LSandbox);
		} catch (std::exception& error) {
			throw this->_logger->error("{}", error.what());
		}
	}

	// UTILS ----
	const std::string& Mod::getID() const { return this->_id; }
	const std::string Mod::getEntryFilePath() const { return fmt::format("{}/init.luau", this->_folder.generic_string()); }
	const std::filesystem::path& Mod::getFolder() const { return this->_folder; }

	lua_State* Mod::getEnvironment() { return this->_LSandbox; }
	// -----
} // namespace rawrbox
