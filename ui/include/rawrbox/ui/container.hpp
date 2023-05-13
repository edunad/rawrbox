#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.hpp>

#include <array>
#include <memory>

namespace rawrbox {
	class UIBase;
	class UIContainer {
	protected:
		std::weak_ptr<rawrbox::UIContainer> _parent;
		std::weak_ptr<rawrbox::UIContainer> _ref;

		std::vector<std::shared_ptr<UIBase>> _children;

		rawrbox::AABBf _aabb = {};
		float _uiScale = 1.F;

	public:
		virtual ~UIContainer() = default;
		UIContainer() = default;

		// UTILS ---
		virtual void setPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getPos() const;
		virtual void setSize(const rawrbox::Vector2f& size);
		[[nodiscard]] virtual const rawrbox::Vector2f getSize() const;
		virtual void setUIScale(float size);
		[[nodiscard]] virtual const float getUIScale() const;

		virtual void removeChildren();
		virtual void remove();
		// ---

		// REFERENCE HANDLING --
		void setRef(const std::shared_ptr<UIContainer>& ref);

		template <class ChildT = UIContainer>
		[[nodiscard]] std::shared_ptr<ChildT> getRef() const {
			return std::dynamic_pointer_cast<ChildT>(this->_ref.lock());
		}

		template <class ChildT = UIContainer>
		[[nodiscard]] std::shared_ptr<ChildT> getParent() const {
			return std::dynamic_pointer_cast<ChildT>(this->_parent.lock());
		}
		// ---

		// CHILD HANDLING --
		template <class T = UIContainer, typename... CallbackArgs>
		std::shared_ptr<T> createChild(CallbackArgs&&... args) {
			auto child = std::make_shared<T>(std::forward<CallbackArgs>(args)...);
			child->setParent(this);

			return child;
		}

		std::vector<std::shared_ptr<UIBase>>& getChildren();
		[[nodiscard]] const std::vector<std::shared_ptr<UIBase>>& getChildren() const;

		virtual void addChild(const std::shared_ptr<UIBase>& elm);
		virtual void setParent(const std::shared_ptr<UIContainer>& elm);
		[[nodiscard]] virtual bool hasParent() const;
		// ----

		// RENDERING -----
		virtual void update();
		virtual void upload();
		virtual void draw(rawrbox::Stencil& stencil);
		// ----
	};
} // namespace rawrbox
