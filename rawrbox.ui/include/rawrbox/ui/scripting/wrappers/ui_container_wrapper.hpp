#pragma once

#include <rawrbox/math/vector2.hpp>

#include <sol/sol.hpp>

namespace rawrbox {
	class UIContainer;

	class UIContainerWrapper {
	protected:
		std::weak_ptr<rawrbox::UIContainer> _ref;

	public:
		UIContainerWrapper(const std::shared_ptr<rawrbox::UIContainer>& ref);
		UIContainerWrapper(const UIContainerWrapper&) = default;
		UIContainerWrapper(UIContainerWrapper&&) = default;
		UIContainerWrapper& operator=(const UIContainerWrapper&) = default;
		UIContainerWrapper& operator=(UIContainerWrapper&&) = default;
		virtual ~UIContainerWrapper();

		[[nodiscard]] virtual rawrbox::UIContainer* getRef() const;

		// UTILS ---
		virtual void setPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f getPos() const;
		[[nodiscard]] virtual const rawrbox::Vector2f getDrawOffset() const;

		virtual void setSize(const rawrbox::Vector2f& size);
		[[nodiscard]] virtual const rawrbox::Vector2f getSize() const;

		virtual void removeChildren();
		virtual void remove();

		virtual void setVisible(bool visible);
		[[nodiscard]] virtual bool visible() const;
		// -----------

		// SORTING -----
		[[nodiscard]] virtual bool alwaysOnTop() const;
		virtual void setAlwaysTop(bool top);
		virtual void bringToFront();
		// -------

		[[nodiscard]] virtual bool isValid() const;
		static void registerLua(sol::state& lua);
	};
} // namespace rawrbox
