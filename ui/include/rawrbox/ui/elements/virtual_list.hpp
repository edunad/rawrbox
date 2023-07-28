
#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <functional>

namespace rawrbox {
	template <class T>
	class UIVirtualList : public rawrbox::UIContainer {
		std::vector<T> items;

		// MOUSE POS ---
		rawrbox::Vector2i mousePos = {-1, -1};
		int hoverIndex = -1;
		//

		// MOUSE SCROLL ----
		int mouseScrollY = 0;
		int maxYOffset = 0;
		// ----

		rawrbox::Color backgroundColor = rawrbox::Colors::Transparent;

	public:
		UIVirtualList() = default;
		UIVirtualList(const UIVirtualList&) = delete;
		UIVirtualList(UIVirtualList&&) = delete;
		UIVirtualList& operator=(const UIVirtualList&) = delete;
		UIVirtualList& operator=(UIVirtualList&&) = delete;
		~UIVirtualList() override = default;

		std::function<bool(size_t, T&)> shouldRender = nullptr;
		std::function<void(size_t, T&, bool, rawrbox::Stencil&)> renderItem = nullptr;
		std::function<void(rawrbox::Stencil&)> renderAfter = nullptr;
		std::function<int(size_t)> getItemSize = nullptr;

		rawrbox::Event<size_t, T&> onItemClick;

		void clear() {
			this->items.clear();
			this->mouseScrollY = 0;
		};

		void addItem(T item) {
			this->items.push_back(item);
		};

		void setItems(std::vector<T> items) {
			this->items = items;
			this->mouseScrollY = 0;
		};

		void resetScroll() {
			this->mouseScrollY = 0;
		};

		bool hasItem(size_t index) {
			return std::find(items.begin(), items.end(), index) != items.end();
		};

		T& getItem(size_t index) {
			return items[index];
		};

		[[nodiscard]] bool clipOverflow() const override { return true; }
		void draw(rawrbox::Stencil& stencil) override {
			if (getItemSize == nullptr) throw std::runtime_error("[VirtualList] Missing 'getItemSize' implementation");
			if (renderItem == nullptr) throw std::runtime_error("[VirtualList] Missing 'renderItem' implementation");

			auto size = this->getSize();
			int yPosOffset = 0;

			// Reset
			hoverIndex = -1;

			// Background
			stencil.drawBox({}, size, backgroundColor);

			for (size_t indx = 0; indx < this->items.size(); indx++) {
				auto itm = this->items[indx];

				if (shouldRender != nullptr && !shouldRender(indx, itm)) continue;
				int itemHeight = this->getItemSize(indx);

				// Item size
				int yStart = yPosOffset + mouseScrollY;
				int yEnd = yPosOffset + itemHeight + mouseScrollY;

				// Hover check
				if (mousePos.y > yStart && mousePos.y < yEnd && mousePos.x > 0 && mousePos.x < size.x) hoverIndex = static_cast<int>(indx);

				// Calculate visibility
				if (yEnd > 0 && yStart < size.y) {
					stencil.pushOffset({0, static_cast<float>(yStart)});
					renderItem(indx, itm, hoverIndex == static_cast<int>(indx), stencil);
					stencil.popOffset();
				}

				// Next item
				yPosOffset += itemHeight + 1; // + offset
			}

			// Set the max Y
			int val = -(yPosOffset - static_cast<int>(size.y));
			if (yPosOffset < size.y) val = 0; // No need to scroll, it's not outside

			maxYOffset = val;
		};

		void afterDraw(rawrbox::Stencil& stencil) override {
			if (renderAfter != nullptr) renderAfter(stencil);
		}

		void mouseUp(const rawrbox::Vector2i& /*mousePos*/, uint32_t button, uint32_t /*mods*/) override {
			if (button != 0 || hoverIndex < 0 || hoverIndex > static_cast<int>(this->items.size())) return;
			this->onItemClick(hoverIndex, this->items[hoverIndex]);
		}

		void mouseMove(const rawrbox::Vector2i& mousePos) override {
			this->mousePos = mousePos;
		}

		void mouseScroll(const rawrbox::Vector2i& /*mousePos*/, const rawrbox::Vector2i& offset) override {
			if (offset.y == 0) return;

			int newVal = mouseScrollY + offset.y * 2;
			if (newVal > 0) newVal = 0;
			if (newVal < maxYOffset) newVal = maxYOffset;

			mouseScrollY = newVal;
		};

		void setHovering(bool hover) override {
			rawrbox::UIContainer::setHovering(hover);

			if (!hover) {
				this->mousePos = {-1, -1};
				this->hoverIndex = -1;
			}
		};

		virtual void setBackgroundColor(rawrbox::Color color) {
			this->backgroundColor = color;
		}

		[[nodiscard]] virtual const rawrbox::Color& getBackgroundColor() const {
			return this->backgroundColor;
		}
	};
} // namespace rawrbox
