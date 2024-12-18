#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/utils/logger.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	class Stencil;
	class UIRoot;
	class UIContainer {

	protected:
		bool _hovering = false;
		bool _focused = false;
		bool _visible = true;

		rawrbox::UIContainer* _parent = nullptr;
		rawrbox::UIRoot* _root = nullptr;

		std::vector<std::shared_ptr<rawrbox::UIContainer>> _children = {};
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-UIContainer");

		bool _alwaysOnTop = false;
		rawrbox::AABBf _aabb = {};

		void internalUpdate(rawrbox::UIContainer* elm);
		void internalDraw(rawrbox::UIContainer* elm, rawrbox::Stencil& stencil);

	public:
		virtual ~UIContainer() = default;

		UIContainer(rawrbox::UIRoot* root);
		UIContainer(const UIContainer&) = delete;
		UIContainer(UIContainer&&) noexcept;
		UIContainer& operator=(const UIContainer&) = delete;
		UIContainer& operator=(UIContainer&&) = delete;

		// UTILS ---
		virtual void setPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getPos() const;
		[[nodiscard]] virtual rawrbox::Vector2f getDrawOffset() const;

		virtual void setSize(const rawrbox::Vector2f& size);
		[[nodiscard]] virtual const rawrbox::Vector2f& getSize() const;
		[[nodiscard]] virtual rawrbox::Vector2f getContentSize() const;

		virtual void sizeToParent();

		virtual void removeChildren();
		virtual void remove();

		virtual void setVisible(bool visible);
		[[nodiscard]] virtual bool visible() const;

		virtual void focus();
		virtual void setFocused(bool focused);
		[[nodiscard]] virtual bool focused() const;

		virtual void setHovering(bool hovering);
		[[nodiscard]] virtual bool hovering() const;

		[[nodiscard]] virtual rawrbox::UIRoot* getRoot() const;
		[[nodiscard]] virtual rawrbox::Vector2f getPosAbsolute() const;
		// ---

		// SORTING -----
		[[nodiscard]] virtual bool alwaysOnTop() const;
		virtual void setAlwaysTop(bool top);
		virtual void bringToFront();
		// -------

		// CHILD HANDLING --
		template <class T, typename... CallbackArgs>
			requires(std::derived_from<T, rawrbox::UIContainer>)
		T* createChild(CallbackArgs&&... args) {
			auto elm = std::make_shared<T>(this->_root, std::forward<CallbackArgs>(args)...);
			elm->setParent(this);

			auto& childn = this->getChildren();
			childn.push_back(std::move(elm));

			return dynamic_cast<T*>(childn.back().get());
		}

		std::vector<std::shared_ptr<rawrbox::UIContainer>>& getChildren();
		[[nodiscard]] const std::vector<std::shared_ptr<rawrbox::UIContainer>>& getChildren() const;

		[[nodiscard]] virtual bool hasChildren() const;
		[[nodiscard]] virtual bool hasParent() const;
		[[nodiscard]] virtual rawrbox::UIContainer* getParent() const;
		virtual void setParent(rawrbox::UIContainer* elm);
		virtual void setRoot(rawrbox::UIRoot* elm);
		// ----

		// RENDERING -----
		virtual void beforeDraw(rawrbox::Stencil& stencil);
		virtual void afterDraw(rawrbox::Stencil& stencil);
		virtual void drawChildren(rawrbox::Stencil& stencil);
		virtual void draw(rawrbox::Stencil& stencil);
		[[nodiscard]] virtual bool clipOverflow() const;
		// ----

		// FOCUS HANDLING ------
		[[nodiscard]] virtual bool lockKeyboard() const;
		[[nodiscard]] virtual bool lockScroll() const;
		[[nodiscard]] virtual bool lockMouse() const;
		[[nodiscard]] virtual bool hitTest(const rawrbox::Vector2f& point) const;
		// --

		// INPUTS ----
		virtual void mouseDown(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods);
		virtual void mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods);
		virtual void mouseScroll(const rawrbox::Vector2i& mousePos, const rawrbox::Vector2i& offset);
		virtual void mouseMove(const rawrbox::Vector2i& mousePos);

		virtual void key(uint32_t key, uint32_t scancode, uint32_t action, uint32_t mods);
		virtual void keyChar(uint32_t key);
		// ---

		virtual void update();
		virtual void updateChildren();
		// ---------
	};
} // namespace rawrbox
