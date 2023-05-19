#pragma once

#include <rawrbox/ui/base.hpp>

#include <array>
#include <memory>

namespace rawrbox {
	class Window;
	class UIRoot : public rawrbox::UIContainer {
	protected:
		std::weak_ptr<rawrbox::UIBase> _focusedElement;
		std::weak_ptr<rawrbox::UIBase> _hoveredElement;

		// INTERNAL UTILS
		int _pressingMouseButton = 0;

		std::shared_ptr<rawrbox::UIBase> findElement(const rawrbox::Vector2i& mousePos, rawrbox::Vector2i& offsetOut);
		std::shared_ptr<rawrbox::UIBase> findElement(std::shared_ptr<rawrbox::UIBase> elmPtr, const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset, rawrbox::Vector2i& offsetOut);
		// ----

		// EVENTS ---
		void onMousePress(const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods);
		void onMouseMove(const rawrbox::Vector2i& location);
		// -----

	public:
		~UIRoot() override;
		UIRoot() = default;

		static std::shared_ptr<rawrbox::UIRoot> create(rawrbox::Window& window);

		[[nodiscard]] const std::shared_ptr<rawrbox::UIBase> getFocus() const;
		void setFocus(std::shared_ptr<rawrbox::UIBase> elm);
	};
} // namespace rawrbox
