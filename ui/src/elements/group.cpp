#include <rawrbox/ui/elements/group.hpp>

namespace rawrbox {
	void UIGroup::sizeToContents() {
		Vector2 totalSize = {0, 0};

		for (auto& child : getChildren()) {
			auto& pos = child->getPos();
			auto& size = child->getSize();

			if (totalSize.x < pos.x + size.x) totalSize.x = pos.x + size.x;
			if (totalSize.y < pos.y + size.y) totalSize.y = pos.y + size.y;
		}

		this->setSize(totalSize);
	}
} // namespace rawrbox
