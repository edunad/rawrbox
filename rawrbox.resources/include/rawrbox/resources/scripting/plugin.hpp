#pragma once

#include <rawrbox/resources/scripting/wrappers/resources_wrapper.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class ResourcesPlugin : public rawrbox::ScriptingPlugin {
	public:
		void registerTypes(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Invalid lua state");
			rawrbox::ResourcesWrapper::registerLua(L);
		}
	};
} // namespace rawrbox
