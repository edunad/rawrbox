#pragma once

#include <rawrbox/scripting/scripting.hpp>
#include <rawrbox/ui/root.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_wrapper.hpp>

namespace rawrbox {
	class UIPlugin : public rawrbox::Plugin {
	protected:
		rawrbox::UIRoot* _root = nullptr;

	public:
		UIPlugin(rawrbox::UIRoot* root) : rawrbox::Plugin(), _root(root){};

		void registerTypes(sol::state& lua) override {
			rawrbox::UIWrapper::registerLua(lua);
			rawrbox::UIContainerWrapper::registerLua(lua);

			// ELEMENTS ----
			rawrbox::FrameWrapper::registerLua(lua);
			rawrbox::ButtonWrapper::registerLua(lua);
			rawrbox::GroupWrapper::registerLua(lua);
			rawrbox::ImageWrapper::registerLua(lua);
			rawrbox::InputWrapper::registerLua(lua);
			rawrbox::LabelWrapper::registerLua(lua);
			rawrbox::ProgressBarWrapper::registerLua(lua);
			rawrbox::GraphCatWrapper::registerLua(lua);
			rawrbox::GraphWrapper::registerLua(lua);
			// ------
		}

		void registerGlobal(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-UIPlugin] Tried to register plugin on invalid mod!");
			if (this->_root == nullptr) throw std::runtime_error("[RawrBox-UIPlugin] Invalid UI root");

			auto& env = mod->getEnvironment();
			env["ui"] = rawrbox::UIWrapper(mod, this->_root);
		}

		void loadLuaExtensions(rawrbox::Mod* mod) override {
			if (mod == nullptr) throw std::runtime_error("[RawrBox-UIPlugin] Tried to register plugin on invalid mod!");
			if (this->_root == nullptr) throw std::runtime_error("[RawrBox-UIPlugin] Invalid UI root");

			auto& env = mod->getEnvironment();

			rawrbox::SCRIPTING::loadLuaFile("./lua/ui.lua", env);
			rawrbox::SCRIPTING::loadLuaFile("./lua/ui_graph_enums.lua", env);
		}
	};
} // namespace rawrbox
