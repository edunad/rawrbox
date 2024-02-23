
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
		std::vector<T> _items;
		rawrbox::VirtualListMode _mode = rawrbox::VirtualListMode::LIST;

		// MOUSE POS ---
		rawrbox::Vector2i _mousePos = {-1, -1};
		int _hoverIndex = -1;
		int _padding = 1;
		// -----

		// MOUSE SCROLL ----
		int _mouseScrollY = 0;
		int _maxYOffset = 0;
		// ----

		rawrbox::Color _backgroundColor = rawrbox::Colors::Transparent();

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
		std::function<rawrbox::Vector2i(size_t)> getItemSize = nullptr;

		rawrbox::Event<size_t, T&, bool> onItemClick;

		[[nodiscard]] const rawrbox::VirtualListMode& getMode() const { return this->_mode; }
		void setMode(rawrbox::VirtualListMode mode) {
			this->_mode = mode;
		}

		[[nodiscard]] bool lockScroll() const override { return true; }

		void clear() {
			this->_items.clear();
			this->_mouseScrollY = 0;
		};

		void addItem(T item) {
			this->_items.push_back(item);
		};

		void setItems(std::vector<T> items) {
			this->_items = items;
			this->_mouseScrollY = 0;
		};

		void resetScroll() {
			this->_mouseScrollY = 0;
		};

		bool hasItem(size_t index) {
			return std::find(this->_items.begin(), this->_items.end(), index) != this->_items.end();
		};

		std::vector<T>& getItems() { return this->_items; }
		T& getItem(size_t index) {
			return this->_items[index];
		};

		int getPadding() { return this->_padding; }
		void setPadding(int padding) {
			this->_padding = padding;
		}

		[[nodiscard]] bool clipOverflow() const override { return true; }
		void draw(rawrbox::Stencil& stencil) override {
			if (this->getItemSize == nullptr) throw std::runtime_error("[VirtualList] Missing 'getItemSize' implementation");
			if (this->renderItem == nullptr) throw std::runtime_error("[VirtualList] Missing 'renderItem' implementation");

			auto size = this->getSize();
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

				// Item size
				int yStart = yPosOffset + this->_mouseScrollY;
				int yEnd = yPosOffset + itemSize.y + this->_mouseScrollY;
				int xEnd = xPosOffset + itemSize.x;

				// Hover check
				if (_mousePos.x > xPosOffset && _mousePos.x < xEnd && _mousePos.y > yStart && _mousePos.y < yEnd) {
					this->_hoverIndex = static_cast<int>(indx);
				}

				// Calculate visibility
				if (yEnd > 0 && yStart < size.y) {
					stencil.pushOffset({static_cast<float>(xPosOffset), static_cast<float>(yStart)});
					this->renderItem(indx, itm, _hoverIndex == static_cast<int>(indx), stencil);
					stencil.popOffset();
				}

				// Next item
				int nextX = itemSize.x + this->_padding;
				int nextY = itemSize.y + this->_padding;

				if (this->_mode == rawrbox::VirtualListMode::GRID) {
					if ((xPosOffset + nextX) >= size.x) {
						yPosOffset += nextY;
						xPosOffset = 0;
					} else {
						xPosOffset += nextX;
					}
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
			if (button != 0 || _hoverIndex < 0 || _hoverIndex > static_cast<int>(this->_items.size())) return;
			this->onItemClick(_hoverIndex, this->_items[_hoverIndex], false);
		}

		void mouseDown(const rawrbox::Vector2i& /*mousePos*/, uint32_t button, uint32_t /*mods*/) override {
			if (button != 0 || _hoverIndex < 0 || _hoverIndex > static_cast<int>(this->_items.size())) return;
			this->onItemClick(_hoverIndex, this->_items[_hoverIndex], true);
		}

		void mouseMove(const rawrbox::Vector2i& pos) override {
			this->_mousePos = pos;
		}

		void mouseScroll(const rawrbox::Vector2i& /*mousePos*/, const rawrbox::Vector2i& offset) override {
			if (offset.y == 0) return;

			int newVal = _mouseScrollY + offset.y * 2;
			if (newVal > 0) newVal = 0;
			if (newVal < _maxYOffset) newVal = _maxYOffset;

			_mouseScrollY = newVal;
		};

		void setHovering(bool hover) override {
			rawrbox::UIContainer::setHovering(hover);

			if (!hover) {
				this->_mousePos = {-1, -1};
				this->_hoverIndex = -1;
			}
		};

		virtual void setBackgroundColor(rawrbox::Color color) {
			this->_backgroundColor = color;
		}

		[[nodiscard]] virtual const rawrbox::Color& getBackgroundColor() const {
			return this->_backgroundColor;
		}
	};
} // namespace rawrbox
