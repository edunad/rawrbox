#pragma once

#include <rawrbox/scripting/plugin.hpp>
#include <rawrbox/ui/root.hpp>
#include <rawrbox/ui/scripting/wrappers/button.hpp>
#include <rawrbox/ui/scripting/wrappers/container.hpp>
#include <rawrbox/ui/scripting/wrappers/frame.hpp>
#include <rawrbox/ui/scripting/wrappers/graph.hpp>
#include <rawrbox/ui/scripting/wrappers/graph_cat.hpp>
#include <rawrbox/ui/scripting/wrappers/group.hpp>
#include <rawrbox/ui/scripting/wrappers/image.hpp>
#include <rawrbox/ui/scripting/wrappers/input.hpp>
#include <rawrbox/ui/scripting/wrappers/label.hpp>
#include <rawrbox/ui/scripting/wrappers/progress_bar.hpp>
#include <rawrbox/ui/scripting/wrappers/tab.hpp>
#include <rawrbox/ui/scripting/wrappers/tabs.hpp>
#include <rawrbox/ui/scripting/wrappers/ui.hpp>

namespace rawrbox {
	class UIPlugin : public rawrbox::ScriptingPlugin {
	protected:
		rawrbox::UIRoot* _root = nullptr;

	public:
		UIPlugin(rawrbox::UIRoot* root) : _root(root){};

		void registerTypes(lua_State* L) override {
			rawrbox::UIContainerWrapper::registerLua(L);

			// ELEMENTS ----
			rawrbox::UIFrameWrapper::registerLua(L);
			rawrbox::UIButtonWrapper::registerLua(L);
			rawrbox::UIGroupWrapper::registerLua(L);
			rawrbox::UIImageWrapper::registerLua(L);
			rawrbox::UIInputWrapper::registerLua(L);
			rawrbox::UILabelWrapper::registerLua(L);
			rawrbox::UIProgressBarWrapper::registerLua(L);
			rawrbox::UIGraphCatWrapper::registerLua(L);
			rawrbox::UIGraphWrapper::registerLua(L);
			rawrbox::UITabWrapper::registerLua(L);
			rawrbox::UITabsWrapper::registerLua(L);
			// ------

			rawrbox::UIWrapper::registerLua(L, _root);
		}

		void loadLibraries(lua_State* L) override {
			if (L == nullptr) throw std::runtime_error("Tried to register plugin on invalid mod!");
			rawrbox::LuaUtils::compileAndLoadFile(L, "RawrBox::Enums::UI", "./lua/enums/ui.lua");
		}
	};
} // namespace rawrbox
