#pragma once

#include <rawrbox/resources/scripting/resources_wrapper.hpp>
#include <rawrbox/scripting/plugin.hpp>

namespace rawrbox {
	class ResourcesPlugin : public rawrbox::Plugin {
	public:
		void registerTypes(sol::state& lua) override {
			rawrbox::ResourcesWrapper::registerLua(lua);
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-ResourcesPlugin] Tried to register plugin on invalid mod!");

			auto& env = mod->getEnvironment();
			env["resources"] = rawrbox::ResourcesWrapper(mod);
		}
	};
} // namespace rawrbox
