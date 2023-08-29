
#include <rawrbox/ui/elements/frame.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/image.hpp>
#include <rawrbox/ui/elements/input.hpp>
#include <rawrbox/ui/elements/label.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/root.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_wrapper.hpp>

namespace rawrbox {
	UIWrapper::UIWrapper(rawrbox::Mod* mod, rawrbox::UIRoot* root) : _mod(mod), _root(root) {}

	// CREATE ------
	sol::object UIWrapper::createFrame(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIFrame>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIFrame>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createGroup(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIGroup>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIGroup>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createButton(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIButton>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIButton>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createProgressBar(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIProgressBar>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIProgressBar>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createLabel(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UILabel>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UILabel>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createInput(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIInput>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIInput>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createImage(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIImage>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIImage>()->getScriptingWrapper();
		}
	}

	sol::object UIWrapper::createGraph(sol::optional<rawrbox::UIContainerWrapper> parent) {
		if (parent.has_value()) {
			return parent->getRef()->createChild<rawrbox::UIGraph>()->getScriptingWrapper();
		} else {
			return this->_root->createChild<rawrbox::UIGraph>()->getScriptingWrapper();
		}
	}
	// ----------------

	// UTILS -----------
	[[nodiscard]] const rawrbox::Vector2f UIWrapper::size() const {
		if (this->_root == nullptr) throw std::runtime_error("[RawrBox-UIWrapper] Invalid UI root reference!");
		return this->_root->getAABB().size;
	}
	// ----------------

	void UIWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<UIWrapper>("ui",
		    sol::no_constructor,
		    "size", &UIWrapper::size,

		    // CREATE ------
		    "createFrame", &UIWrapper::createFrame,
		    "createGroup", &UIWrapper::createGroup,
		    "createButton", &UIWrapper::createButton,
		    "createProgressBar", &UIWrapper::createProgressBar,
		    "createLabel", &UIWrapper::createLabel,
		    "createInput", &UIWrapper::createInput,
		    "createImage", &UIWrapper::createImage,
		    "createGraph", &UIWrapper::createGraph,
		    // -----

		    // Overwritten by lua
		    "create", &UIWrapper::create);
	}
} // namespace rawrbox
