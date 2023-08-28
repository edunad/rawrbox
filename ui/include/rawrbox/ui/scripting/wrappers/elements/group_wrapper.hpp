#pragma once

#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class GroupWrapper : public rawrbox::UIContainerWrapper {

	public:
		GroupWrapper(const std::shared_ptr<rawrbox::UIContainer>& element);

		virtual void setBorder(float border);
		[[nodiscard]] virtual float getBorder() const;

		virtual void sizeToContents();

		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
