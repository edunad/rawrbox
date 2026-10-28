#include <rawrbox/render/scripting/global/loading.hpp>
#include <rawrbox/render/utils/loading.hpp>

namespace rawrbox {
	void LoadingGlobal::add(const std::string& id, const std::string& title, size_t done, size_t total, const std::optional<std::string>& detail) {
		rawrbox::LoadingUtils::add(id, title, done, total, detail.value_or(""));
	}

	void LoadingGlobal::remove(const std::string& id) {
		rawrbox::LoadingUtils::remove(id);
	}

	void LoadingGlobal::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("loading", {})
		    .addFunction("add", &LoadingGlobal::add)
		    .addFunction("remove", &LoadingGlobal::remove)
		    .addFunction("hasTasks", &rawrbox::LoadingUtils::hasTasks)
		    .endNamespace();
	}
} // namespace rawrbox