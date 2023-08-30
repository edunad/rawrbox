#pragma once

#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class GroupWrapper : public rawrbox::UIContainerWrapper {

	public:
		using UIContainerWrapper::UIContainerWrapper;

		virtual void setBorder(float border);
		[[nodiscard]] virtual float getBorder() const;

		virtual void sizeToContents();

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
