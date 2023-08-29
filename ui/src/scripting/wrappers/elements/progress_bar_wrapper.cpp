#include <rawrbox/ui/elements/progress_bar.hpp>
#include <rawrbox/ui/scripting/wrappers/elements/progress_bar_wrapper.hpp>
#include <rawrbox/utils/memory.hpp>

namespace rawrbox {

	// UTILS -----
	void ProgressBarWrapper::showPercent(bool show) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->showPercent(show);
	}

	bool ProgressBarWrapper::isPercentVisible() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->isPercentVisible();
	}

	void ProgressBarWrapper::setBarColor(const rawrbox::Colori& color) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->setBarColor(color.cast<float>());
	}

	rawrbox::Colori ProgressBarWrapper::getBarColor() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->getBarColor().cast<int>();
	}

	void ProgressBarWrapper::setValue(float value) {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->setValue(value);
	}

	float ProgressBarWrapper::getValue() const {
		if (!this->isValid()) throw std::runtime_error("[RawrBox-ProgressBarWrapper] Invalid ui reference");
		return rawrbox::cast<rawrbox::UIProgressBar>(this->_ref).lock()->getValue();
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
