#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/scripting/wrappers/ui_container_wrapper.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class FrameWrapper : public rawrbox::UIContainerWrapper {

	public:
		using UIContainerWrapper::UIContainerWrapper;

		void setTitle(const std::string& title);
		[[nodiscard]] const std::string& getTitle() const;

		void setTitleColor(const rawrbox::Colori& cl);
		[[nodiscard]] rawrbox::Colori getTitleColor() const;

		void setDraggable(bool enabled);
		[[nodiscard]] bool isDraggable() const;

		void setClosable(bool enabled);
		[[nodiscard]] bool isClosable() const;

		void onClose(sol::function onCloseCallback);
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
