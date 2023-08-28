#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/scripting/mod.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class ProgressBarWrapper : public rawrbox::UIContainerWrapper {

	public:
		ProgressBarWrapper(const std::shared_ptr<rawrbox::UIContainer>& element);

		// UTILS -----
		virtual void showPercent(bool show);
		[[nodiscard]] virtual bool isPercentVisible() const;

		virtual void setBarColor(const rawrbox::Colori& color);
		[[nodiscard]] virtual rawrbox::Colori getBarColor() const;

		virtual void setValue(float value);
		[[nodiscard]] virtual float getValue() const;
		// ----

		void onValueChange(sol::function onValueChange);

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
