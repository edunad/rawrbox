#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/vector2.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	class UIBase;
	class Stencil;
	class UIContainer {
	protected:
		std::weak_ptr<rawrbox::UIContainer> _parent;
		std::weak_ptr<rawrbox::UIContainer> _ref;

		std::vector<std::shared_ptr<UIBase>> _children;

		rawrbox::AABBf _aabb = {};

		void internalUpdate(std::shared_ptr<rawrbox::UIBase> elm);
		void internalDraw(std::shared_ptr<rawrbox::UIBase> elm, rawrbox::Stencil& stencil);

	public:
		virtual ~UIContainer() = default;
		UIContainer() = default;

		// UTILS ---
		virtual void setPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f getPos() const;
		[[nodiscard]] virtual const rawrbox::Vector2f getDrawOffset() const;

		virtual void setSize(const rawrbox::Vector2f& size);
		[[nodiscard]] virtual const rawrbox::Vector2f getSize() const;

		virtual void removeChildren();
		virtual void remove();
		// ---

		// REFERENCE HANDLING --
		void setRef(std::shared_ptr<rawrbox::UIContainer> ref);

		template <class ChildT = rawrbox::UIContainer>
		[[nodiscard]] std::shared_ptr<ChildT> getRef() const {
			return std::dynamic_pointer_cast<ChildT>(this->_ref.lock());
		}

		template <class ChildT = rawrbox::UIContainer>
		[[nodiscard]] std::shared_ptr<ChildT> getParent() const {
			return std::dynamic_pointer_cast<ChildT>(this->_parent.lock());
		}
		// ---

		// CHILD HANDLING --
		template <class T, typename... CallbackArgs>
		std::shared_ptr<T> createChild(CallbackArgs&&... args) {
			auto child = std::make_shared<T>(std::forward<CallbackArgs>(args)...);
			child->setRef(child);

			this->addChild(std::dynamic_pointer_cast<rawrbox::UIBase>(child));
			return child;
		}

		std::vector<std::shared_ptr<UIBase>>& getChildren();
		[[nodiscard]] const std::vector<std::shared_ptr<UIBase>>& getChildren() const;

		virtual void addChild(std::shared_ptr<rawrbox::UIBase> elm);
		virtual void setParent(std::shared_ptr<rawrbox::UIContainer> elm);

		[[nodiscard]] virtual bool hasChildren() const;
		[[nodiscard]] virtual bool hasParent() const;
		// ----

		// RENDERING -----
		[[nodiscard]] virtual bool clipOverflow() const;
		virtual void update();
		virtual void draw(rawrbox::Stencil& stencil);
		// ----
	};
} // namespace rawrbox
