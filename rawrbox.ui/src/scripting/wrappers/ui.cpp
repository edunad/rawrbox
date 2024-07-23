#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/graph.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/loading.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/elements/tabs.hpp>
#include <rawrbox/ui/root.hpp>
#include <rawrbox/ui/scripting/wrappers/ui.hpp>

namespace rawrbox {
	void UIWrapper::registerLua(lua_State* L, rawrbox::UIRoot* root) {
		luabridge::getGlobalNamespace(L)
		    .beginNamespace("ui", luabridge::allowOverridingMethods)
		    .addFunction("size", [root]() {
			    return root->getAABB();
		    })
		    .addFunction("createFrame", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIFrame>();
			    }

			    return root->createChild<rawrbox::UIFrame>();
		    })
		    .addFunction("createGroup", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIGroup>();
			    }

			    return root->createChild<rawrbox::UIGroup>();
		    })
		    .addFunction("createButton", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIButton>();
			    }

			    return root->createChild<rawrbox::UIButton>();
		    })
		    .addFunction("createProgressBar", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIProgressBar>();
			    }

			    return root->createChild<rawrbox::UIProgressBar>();
		    })
		    .addFunction("createProgressBar", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIProgressBar>();
			    }

			    return root->createChild<rawrbox::UIProgressBar>();
		    })
		    .addFunction("createProgressBar", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIProgressBar>();
			    }

			    return root->createChild<rawrbox::UIProgressBar>();
		    })
		    .addFunction("createLabel", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UILabel>();
			    }

			    return root->createChild<rawrbox::UILabel>();
		    })
		    .addFunction("createInput", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIInput>();
			    }

			    return root->createChild<rawrbox::UIInput>();
		    })
		    .addFunction("createImage", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIImage>();
			    }

			    return root->createChild<rawrbox::UIImage>();
		    })
		    .addFunction("createGraph", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UIGraph>();
			    }

			    return root->createChild<rawrbox::UIGraph>();
		    })
		    .addFunction("createTabs", [root](std::optional<rawrbox::UIContainer*> parent, const std::vector<rawrbox::UITab>& tabs) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UITabs>(tabs);
			    }

			    return root->createChild<rawrbox::UITabs>(tabs);
		    })
		    .addFunction("createLoading", [root](std::optional<rawrbox::UIContainer*> parent) {
			    if (parent.has_value()) {
				    return parent.value()->createChild<rawrbox::UILoading>();
			    }

			    return root->createChild<rawrbox::UILoading>();
		    })
		    .endNamespace();
	}
} // namespace rawrbox
