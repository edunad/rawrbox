
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/utils/path.hpp>

namespace rawrbox {
	Mod::Mod(std::string id, std::filesystem::path folderPath, glz::generic metadata) : _L(luaL_newstate()), _modTable(_L), _folder(std::move(folderPath)), _id(std::move(id)), _metadata(std::move(metadata)) {}
	Mod::~Mod() {
		this->gc();
		this->_L = nullptr;
	}

	void Mod::shutdown() {
		if (this->_L == nullptr) RAWRBOX_CRITICAL("Invalid lua handle");
		this->call("onShutdown");
	}

	void Mod::init() {
		if (this->_L == nullptr) RAWRBOX_CRITICAL("Invalid lua handle");

#ifdef RAWRBOX_SCRIPTING_EXCEPTION
		luabridge::enableExceptions(_L);
#endif

		// Inject mod env --
		lua_pushstring(this->_L, this->_folder.generic_string().c_str());
		lua_setglobal(this->_L, "__mod_folder");

		lua_pushstring(this->_L, this->_folder.filename().generic_string().c_str());
		lua_setglobal(this->_L, "__mod_folder_root");

		lua_pushstring(this->_L, this->_id.c_str());
		lua_setglobal(this->_L, "__mod_id");
		// -----------------

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
		if (this->_L == nullptr) RAWRBOX_CRITICAL("Invalid lua handle");
		rawrbox::LuaUtils::collect_garbage(this->_L);
	}

	void Mod::load() {
		if (this->_L == nullptr) RAWRBOX_CRITICAL("Invalid lua sandbox environment");
		rawrbox::LuaUtils::compileAndLoadFile(this->_L, this->getID(), this->getEntryFilePath());
	}

	void Mod::script(const std::string& script) {
		if (this->_L == nullptr) RAWRBOX_CRITICAL("Invalid lua sandbox environment");
		rawrbox::LuaUtils::compileAndLoadScript(this->_L, "unknown", script);
	}

	// UTILS ----
	const std::string& Mod::getID() const { return this->_id; }

	std::string Mod::getEntryFilePath() const { return fmt::format("{}/init.luau", this->_folder.generic_string()); }
	const std::filesystem::path& Mod::getFolder() const { return this->_folder; }

	const glz::generic& Mod::getMetadata() const { return this->_metadata; }

	lua_State* Mod::getEnvironment() { return this->_L; }

#ifdef RAWRBOX_SCRIPTING_WORKSHOP_MODDING
	void Mod::setWorkshopId(uint64_t id) { this->_workshopId = id; }
	uint64_t Mod::getWorkshopId() const { return this->_workshopId; }

	void Mod::setWorkshopType(uint32_t type) { this->_workshopType = type; }
	uint32_t Mod::getWorkshopType() const { return this->_workshopType; }
#endif
	// -----
} // namespace rawrbox
