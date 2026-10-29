#include <rawrbox/scripting/wrappers/profiler.hpp>

#include <tracy/Tracy.hpp>

namespace rawrbox {
	// PRIVATE -----
	std::unordered_set<std::string> ProfilerWrapper::_traces = {};
	// -------------

	void ProfilerWrapper::zone([[maybe_unused]] const std::string& name, const luabridge::LuaRef& callback) {
		if (!callback.isCallable()) throw std::runtime_error("Invalid callback");
		ZoneTransientN(luaZone, name.c_str(), true);

		auto result = rawrbox::LuaUtils::call(callback);
		if (result.hasFailed()) throw std::runtime_error(result.errorMessage());
	}

	void ProfilerWrapper::plot([[maybe_unused]] const std::string& name, [[maybe_unused]] double value) {
#ifdef TRACY_ENABLE
		const auto& id = *_traces.insert(name).first;
		TracyPlot(id.c_str(), value);
#endif
	}

	void ProfilerWrapper::message([[maybe_unused]] const std::string& text) {
		TracyMessage(text.c_str(), text.size());
	}

	void ProfilerWrapper::registerLua(lua_State* L) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("profiler", {})
		    .addFunction("zone", &rawrbox::ProfilerWrapper::zone)
		    .addFunction("plot", &rawrbox::ProfilerWrapper::plot)
		    .addFunction("message", &rawrbox::ProfilerWrapper::message)
		    .endNamespace();
	}
} // namespace rawrbox
