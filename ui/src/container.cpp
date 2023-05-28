
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/base.hpp>
#include <rawrbox/ui/container.hpp>

namespace rawrbox {

	// UTILS ---
	const rawrbox::Vector2f UIContainer::getDrawOffset() const { return {}; };

	void UIContainer::setPos(const rawrbox::Vector2f& pos) { this->_aabb.pos = pos; }
	const rawrbox::Vector2f& UIContainer::getPos() const { return this->_aabb.pos; }

	void UIContainer::setSize(const rawrbox::Vector2f& size) { this->_aabb.size = size; }
	const rawrbox::Vector2f UIContainer::getSize() const {
		return {this->_aabb.size.x * this->_uiScale, this->_aabb.size.y * this->_uiScale};
	}

	void UIContainer::setUIScale(const float scale) { this->_uiScale = scale; }
	const float UIContainer::getUIScale() const { return this->_uiScale; }

	void UIContainer::removeChildren() {
		while (!this->_children.empty())
			this->_children.front()->remove();
	}

	void UIContainer::remove() {
		this->removeChildren();

		auto p = this->_parent.lock();
		auto& parentChilds = p->getChildren();
		auto sharedPtr = getRef<UIBase>();
		parentChilds.erase(std::find(parentChilds.begin(), parentChilds.end(), sharedPtr));

		this->_parent.reset();
	}
	// ---

	// REFERENCE HANDLING --
	void UIContainer::setRef(std::shared_ptr<UIContainer> ref) { this->_ref = ref; }
	// ---

	// PARENTING ---
	std::vector<std::shared_ptr<rawrbox::UIBase>>& UIContainer::getChildren() { return this->_children; }
	const std::vector<std::shared_ptr<rawrbox::UIBase>>& UIContainer::getChildren() const { return this->_children; }

	bool UIContainer::hasParent() const { return !this->_parent.expired(); }
	void UIContainer::addChild(std::shared_ptr<rawrbox::UIBase> elm) {
		elm->setParent(this->getRef<rawrbox::UIContainer>());
	}

	void UIContainer::setParent(std::shared_ptr<rawrbox::UIContainer> elm) {
		auto sharedPtr = this->getRef<rawrbox::UIBase>();

		if (this->hasParent()) {
			auto& childs = this->_parent.lock()->getChildren();
			childs.erase(std::find(childs.begin(), childs.end(), sharedPtr));
		}

		this->_parent = elm;

		auto& childn = elm->getChildren();
		if (sharedPtr->alwaysOnTop()) {
			childn.insert(childn.begin(), sharedPtr);
		} else {
			childn.push_back(sharedPtr);
		}
	}
	// --------------

	// RENDERING -----
	void UIContainer::internalUpdate(std::shared_ptr<rawrbox::UIBase> elm) {
		if (!elm->visible()) return;
		elm->update();

		auto elms = elm->getChildren();
		for (auto& celm : elms) {
			this->internalUpdate(celm);
		}
	}

	void UIContainer::update() {
		for (auto elm : this->_children) {
			this->internalUpdate(elm);
		}
	}

	void UIContainer::internalDraw(std::shared_ptr<rawrbox::UIBase> elm, rawrbox::Stencil& stencil) {
		if (!elm->visible()) return;

		stencil.pushOffset(elm->getPos());
		stencil.pushClipping({{}, elm->getSize()});

		elm->beforeDraw(stencil);
		elm->draw(stencil);

		// Draw children of the element ---
		stencil.pushOffset(elm->getDrawOffset());
		auto elms = elm->getChildren();
		for (auto& celm : elms) {
			this->internalDraw(celm, stencil);
		}
		stencil.popOffset();
		// -----------

		elm->afterDraw(stencil);

		stencil.popClipping();
		stencil.popOffset();
	}

	void UIContainer::draw(rawrbox::Stencil& stencil) {
		for (auto elm : this->_children) {
			this->internalDraw(elm, stencil);
		}
	}
	// ----
} // namespace rawrbox
