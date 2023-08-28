#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/group_wrapper.hpp>

#include <fmt/format.h>

namespace rawrbox {
	GroupWrapper::GroupWrapper(const std::shared_ptr<rawrbox::UIContainer>& element) : rawrbox::UIContainerWrapper(element) {}

	void GroupWrapper::setBorder(float border) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGroup> ptr = std::dynamic_pointer_cast<rawrbox::UIGroup>(this->_ref.lock());
		ptr.lock()->setBorder(border);
	}

	float GroupWrapper::getBorder() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGroup> ptr = std::dynamic_pointer_cast<rawrbox::UIGroup>(this->_ref.lock());
		return ptr.lock()->getBorder();
	}

	void GroupWrapper::sizeToContents() {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-GroupWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIGroup> ptr = std::dynamic_pointer_cast<rawrbox::UIGroup>(this->_ref.lock());
		ptr.lock()->sizeToContents();
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
