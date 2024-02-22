#pragma once

#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <memory>

namespace rawrbox {
	class Window;
	class UIRoot {
	protected:
		rawrbox::Window* _window = nullptr;

		std::vector<std::shared_ptr<rawrbox::UIContainer>> _children = {};
		rawrbox::AABBf _aabb = {};

		// INTERNAL UTILS
		int _pressingMouseButton = 0;
		// ----

		// CHILDREN
		void removeChildren();
		// ----

		// EVENTS ---
		void onMousePress(const rawrbox::Vector2i& location, uint32_t button, uint32_t action, uint32_t mods);
		void onMouseMove(const rawrbox::Vector2i& location);
		// -----

	public:
		rawrbox::UIContainer* focusedElement = nullptr;
		rawrbox::UIContainer* hoveredElement = nullptr;

		rawrbox::Event<rawrbox::UIContainer*> onFocusChange;

		explicit UIRoot(rawrbox::Window& window);

		// UTIL
		rawrbox::UIContainer* findElement(const rawrbox::Vector2i& mousePos, rawrbox::Vector2i& offsetOut);
		rawrbox::UIContainer* findElement(rawrbox::UIContainer* elmPtr, const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset, rawrbox::Vector2i& offsetOut);
		[[nodiscard]] const rawrbox::AABBf& getAABB() const;
		// ---

		// CHILDREN
		void removeChild(rawrbox::UIContainer* elm);
		std::vector<std::shared_ptr<rawrbox::UIContainer>>& getChildren();

		template <class T, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::UIContainer>)
		T* createChild(CallbackArgs&&... args) {
			auto elm = std::make_shared<T>(std::forward<CallbackArgs>(args)...);
			elm->setRoot(this);
			elm->initialize();

			auto& childn = this->getChildren();
			if (elm->alwaysOnTop()) {
				childn.insert(childn.begin(), std::move(elm));
			} else {
				childn.push_back(std::move(elm));
			}

			return dynamic_cast<T*>(this->getChildren().back().get());
		}
		// ----

		// FOCUS
		[[nodiscard]] const rawrbox::UIContainer* getFocus() const;
		void setFocus(rawrbox::UIContainer* elm);
		// ----

		void render();
		void update();
	};
} // namespace rawrbox
