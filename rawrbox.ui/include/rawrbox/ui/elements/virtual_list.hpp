
#pragma once
#include <rawrbox/math/color.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <functional>

namespace rawrbox {
	enum class VirtualListMode {
		LIST = 0,
		GRID = 1
	};

	template <class T>
	class UIVirtualList : public rawrbox::UIContainer {
	protected:
		std::vector<T> _items = {};
		rawrbox::VirtualListMode _mode = rawrbox::VirtualListMode::LIST;

		// MOUSE POS ---
		rawrbox::Vector2i _mousePos = {-1, -1};
		int _hoverIndex = -1;
		int _padding = 1;
		// -----

		// MOUSE SCROLL ----
		int _mouseScrollY = 0;
		int _maxYOffset = 0;
		int _scrollSpeed = 2;
		// ----

		rawrbox::Color _backgroundColor = rawrbox::Colors::Transparent();

	public:
		UIVirtualList(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {};
		UIVirtualList(const UIVirtualList&) = delete;
		UIVirtualList(UIVirtualList&&) = delete;
		UIVirtualList& operator=(const UIVirtualList&) = delete;
		UIVirtualList& operator=(UIVirtualList&&) = delete;
		~UIVirtualList() override = default;

		std::function<bool(size_t, T&)> shouldRender = nullptr;
		std::function<void(size_t, T&, bool, rawrbox::Stencil&)> renderItem = nullptr;
		std::function<void(rawrbox::Stencil&)> renderAfter = nullptr;
		std::function<rawrbox::Vector2i(size_t)> getItemSize = nullptr;

		rawrbox::Event<size_t, T&, bool> onItemClick = {};

		[[nodiscard]] const rawrbox::VirtualListMode& getMode() const { return this->_mode; }
		void setMode(rawrbox::VirtualListMode mode) {
			this->_mode = mode;
		}

		[[nodiscard]] bool lockScroll() const override {
			return true;
		}

		void clear() {
			this->_items.clear();
			this->_mouseScrollY = 0;
		};

		void addItem(const T& item) {
			this->_items.emplace_back(std::move(item));
		};

		void addItem(T& item) {
			this->_items.emplace_back(std::move(item));
		};

		void addItem(T&& item) {
			this->_items.emplace_back(std::move(item));
		};

		void removeItem(size_t index) {
			if (index >= this->_items.size()) return;
			this->_items.erase(this->_items.begin() + index);
		};

		void setItems(const std::vector<T>& items) {
			this->_items = items;
			this->_mouseScrollY = 0;
		};

		[[nodiscard]] size_t total() const {
			return this->_items.size();
		};

		[[nodiscard]] bool empty() const {
			return this->_items.empty();
		};

		void resetScroll() {
			this->_mouseScrollY = 0;
		};

		[[nodiscard]] int getScrollSpeed() const { return this->_scrollSpeed; }
		void setScrollSpeed(int speed) {
			this->_scrollSpeed = speed;
		}

		[[nodiscard]] bool hasItem(size_t index) const {
			return index < this->_items.size();
		};

		[[nodiscard]] const std::vector<T>& getItems() const { return this->_items; }

		[[nodiscard]] const T& getItem(size_t index) const {
			if (index >= this->_items.size()) throw std::runtime_error("[VirtualList] Index out of bounds");
			return this->_items[index];
		};

		[[nodiscard]] T& getItem(size_t index) {
			if (index >= this->_items.size()) throw std::runtime_error("[VirtualList] Index out of bounds");
			return this->_items[index];
		};

		[[nodiscard]] int getPadding() const { return this->_padding; }
		void setPadding(int padding) {
			this->_padding = padding;
		}

		[[nodiscard]] bool clipOverflow() const override { return true; }
		void draw(rawrbox::Stencil& stencil) override {
			if (this->getItemSize == nullptr) throw std::runtime_error("[VirtualList] Missing 'getItemSize' implementation");
			if (this->renderItem == nullptr) throw std::runtime_error("[VirtualList] Missing 'renderItem' implementation");

			const auto& size = this->getSize();

			int yPosOffset = 0;
			int xPosOffset = 0;

			// Reset
			this->_hoverIndex = -1;

			// Background
			stencil.drawBox({}, size, this->_backgroundColor);

			for (size_t indx = 0; indx < this->_items.size(); indx++) {
				auto& itm = this->_items[indx];

				if (this->shouldRender != nullptr && !this->shouldRender(indx, itm)) continue;
				auto itemSize = this->getItemSize(indx);

				// Check if item fits
				int nextX = itemSize.x + this->_padding;
				int nextY = itemSize.y + this->_padding;
				bool fits = (xPosOffset + itemSize.x) <= size.x;

				if (this->_mode == rawrbox::VirtualListMode::GRID && !fits) {
					yPosOffset += nextY;
					xPosOffset = 0;
				}

				// Item size
				int yStart = yPosOffset + this->_mouseScrollY;
				int yEnd = yPosOffset + itemSize.y + this->_mouseScrollY;
				int xEnd = xPosOffset + itemSize.x;

				// Hover check
				if (this->_mousePos.x > xPosOffset && this->_mousePos.x<xEnd&& this->_mousePos.y> yStart && this->_mousePos.y < yEnd) {
					this->_hoverIndex = static_cast<int>(indx);
				}

				// Calculate visibility
				if (yEnd > 0 && yStart < size.y) {
					stencil.pushOffset({static_cast<float>(xPosOffset), static_cast<float>(yStart)});
					this->renderItem(indx, itm, this->_hoverIndex == static_cast<int>(indx), stencil);
					stencil.popOffset();
				}

				// Next item
				if (this->_mode == rawrbox::VirtualListMode::GRID) {
					if (fits) xPosOffset += nextX;
				} else {
					yPosOffset += nextY;
				}
				// ------------
			}

			// Set the max Y
			int val = -(yPosOffset - static_cast<int>(size.y));
			if (yPosOffset < size.y) val = 0; // No need to scroll, it's not outside

			this->_maxYOffset = val;
		};

		void afterDraw(rawrbox::Stencil& stencil) override {
			if (renderAfter != nullptr) renderAfter(stencil);
		}

		void mouseUp(const rawrbox::Vector2i& /*mousePos*/, uint32_t button, uint32_t /*mods*/) override {
			if (button != 0 || this->_hoverIndex < 0 || this->_hoverIndex > static_cast<int>(this->_items.size())) return;
			this->onItemClick(this->_hoverIndex, this->_items[this->_hoverIndex], false);
		}

		void mouseDown(const rawrbox::Vector2i& /*mousePos*/, uint32_t button, uint32_t /*mods*/) override {
			if (button != 0 || this->_hoverIndex < 0 || this->_hoverIndex > static_cast<int>(this->_items.size())) return;
			this->onItemClick(this->_hoverIndex, this->_items[this->_hoverIndex], true);
		}

		void mouseMove(const rawrbox::Vector2i& pos) override {
			this->_mousePos = pos;
		}

		void mouseScroll(const rawrbox::Vector2i& /*mousePos*/, const rawrbox::Vector2i& offset) override {
			if (offset.y == 0) return;
			this->_mouseScrollY = std::clamp<int>(this->_mouseScrollY + offset.y * this->_scrollSpeed, this->_maxYOffset, 0);
		};

		void setHovering(bool hover) override {
			rawrbox::UIContainer::setHovering(hover);

			if (!hover) {
				this->_mousePos = {-1, -1};
				this->_hoverIndex = -1;
			}
		};

		virtual void setBackgroundColor(const rawrbox::Color& color) {
			this->_backgroundColor = color;
		}

		[[nodiscard]] virtual const rawrbox::Color& getBackgroundColor() const {
			return this->_backgroundColor;
		}
	};
} // namespace rawrbox
