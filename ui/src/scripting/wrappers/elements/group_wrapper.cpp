#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/group_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {

	void GroupWrapper::setBorder(float border) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGroup>(this->_ref).lock()->setBorder(border);
	}

	float GroupWrapper::getBorder() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIGroup>(this->_ref).lock()->getBorder();
	}

	void GroupWrapper::sizeToContents() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIGroup>(this->_ref).lock()->sizeToContents();
	}

	void GroupWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<GroupWrapper>("UIGroup",
		    sol::no_constructor,

		    "setBorder", &GroupWrapper::setBorder,
		    "getBorder", &GroupWrapper::getBorder,

		    "sizeToContents", &GroupWrapper::sizeToContents,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
