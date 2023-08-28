#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/progress_bar_wrapper.hpp>

namespace rawrbox {
	ProgressBarWrapper::ProgressBarWrapper(const std::shared_ptr<rawrbox::UIContainer>& element) : rawrbox::UIContainerWrapper(element) {}

	// UTILS -----
	void ProgressBarWrapper::showPercent(bool show) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		ptr.lock()->showPercent(show);
	}

	bool ProgressBarWrapper::isPercentVisible() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		return ptr.lock()->isPercentVisible();
	}

	void ProgressBarWrapper::setBarColor(const rawrbox::Colori& color) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		ptr.lock()->setBarColor(color.cast<float>());
	}

	rawrbox::Colori ProgressBarWrapper::getBarColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		return ptr.lock()->getBarColor().cast<int>();
	}

	void ProgressBarWrapper::setValue(float value) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		return ptr.lock()->setValue(value);
	}

	float ProgressBarWrapper::getValue() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		std::weak_ptr<rawrbox::UIProgressBar> ptr = std::dynamic_pointer_cast<rawrbox::UIProgressBar>(this->_ref.lock());

		return ptr.lock()->getValue();
	}
	// ----

	void ProgressBarWrapper::registerLua(sol::state& lua) {
		lua.new_usertype<ProgressBarWrapper>("UIProgressBar",
		    sol::no_constructor,

		    "showPercent", &ProgressBarWrapper::showPercent,
		    "isPercentVisible", &ProgressBarWrapper::isPercentVisible,

		    "setBarColor", &ProgressBarWrapper::setBarColor,
		    "getBarColor", &ProgressBarWrapper::getBarColor,

		    "setValue", &ProgressBarWrapper::setValue,
		    "getValue", &ProgressBarWrapper::getValue,

		    sol::base_classes, sol::bases<rawrbox::UIContainerWrapper>());
	}
} // namespace rawrbox
