#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/dropdown.hpp>
#include <rawrbox/ui/root.hpp>
#include <rawrbox/utils/keys.hpp>

namespace rawrbox {
	// PRIVATE ---
	void UIDropdown::calculateListSize() {
		if (this->_list == nullptr) return;
		float itemSizeY = std::min(static_cast<float>(this->_list->total()) * this->_originalSize.y, this->_originalSize.y * 4.F);

		this->_aabb.size = {this->_originalSize.x, itemSizeY + this->_originalSize.y};
		this->_list->setSize({this->_originalSize.x, itemSizeY});
		this->_list->setPos({0, this->_originalSize.y});
	}
	// -----------

	UIDropdown::UIDropdown(rawrbox::UIRoot* root, const std::vector<std::string>& options) : rawrbox::UIContainer(root), _list(this->createChild<rawrbox::UIVirtualList<std::string>>()) {
		this->_icon = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/ui/icons/dropdown.png")->get();

		this->_list->setScrollSpeed(4);
		this->_list->setItems(options);
		this->_list->setMode(rawrbox::VirtualListMode::LIST);
		this->_list->setBackgroundColor(rawrbox::Colors::Black());
		this->_list->setVisible(false);
		this->_list->getItemSize = [this](size_t /*indx*/) -> rawrbox::Vector2i {
			return this->_originalSize.cast<int>();
		};

		this->_list->renderItem = [this](size_t /*indx*/, const std::string& text, bool isHovering, rawrbox::Stencil& stencil) {
			const auto& charSize = rawrbox::DEBUG_FONT_REGULAR->getCharSize();
			auto trimStr = rawrbox::StrUtils::truncate(text, static_cast<size_t>(this->_originalSize.x / charSize.x) - 6);

			stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, trimStr, {5, 5}, rawrbox::Colors::White());
			if (isHovering) stencil.drawBox({}, this->_originalSize, rawrbox::Color::RGBAHex(0x89898961));
		};

		this->_list->onItemClick += [this](size_t indx, const std::string& /*tex*/, bool isDown) {
			if (!isDown) return;
			this->setActive(indx);
		};
	}

	// UTILS ----
	void UIDropdown::setOptions(const std::vector<std::string>& options) {
		if (this->_list == nullptr) return;
		this->_list->setItems(options);
	}

	void UIDropdown::addOption(const std::string& option) {
		if (this->_list == nullptr) return;

		this->_list->addItem(option);
		this->calculateListSize();
	}

	void UIDropdown::removeOption(size_t index) {
		if (this->_list == nullptr) return;

		this->_list->removeItem(index);
		this->calculateListSize();
	}

	void UIDropdown::setActive(size_t option) {
		if (this->_list == nullptr || option > this->_list->total()) return;

		this->_selected = option;
		this->onSelectionChange(this->_selected, this->_list->getItem(this->_selected));
	}

	size_t UIDropdown::getSelected() const { return this->_selected; }
	const std::string& UIDropdown::getSelectedValue() const { return this->_list->getItem(this->_selected); }
	// ---------

	// OVERRIDE ---
	void UIDropdown::setSize(const rawrbox::Vector2f& size) {
		this->_originalSize = size;
		this->calculateListSize();
	}

	bool UIDropdown::hitTest(const rawrbox::Vector2f& point) const {
		if (this->_list != nullptr && this->_list->visible()) {
			return rawrbox::UIContainer::hitTest(point);
		}

		const rawrbox::AABBf& test = {this->getPos(), this->_originalSize};
		return test.contains(point);
	}

	void UIDropdown::setFocused(bool focused) {
		rawrbox::UIContainer::setFocused(focused);

		this->_list->setVisible(focused);
		if (focused) this->_list->bringToFront();
	}
	// ----------

	// DRAW ----
	void UIDropdown::draw(rawrbox::Stencil& stencil) {
		// BG ---
		stencil.drawBox({}, this->_originalSize, rawrbox::Color::RGBHex(0x36393f));

		stencil.pushOutline({2, 0});
		stencil.drawBox({}, this->_originalSize, rawrbox::Color::RGBAHex(0x0000001A));
		stencil.popOutline();
		// -----

		// TEXT ---
		if (!this->_list->empty()) {
			const auto& selected = this->_list->getItem(this->_selected);
			const auto& charSize = rawrbox::DEBUG_FONT_REGULAR->getCharSize();
			auto trimStr = rawrbox::StrUtils::truncate(selected, static_cast<size_t>(this->_originalSize.x / charSize.x) - 6);

			stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, trimStr, {5, 5}, rawrbox::Colors::White());
		}
		// --------

		// ICON ---
		stencil.drawTexture({this->_originalSize.x - ICON_SIZE, (this->_originalSize.y - ICON_SIZE) / 2.F}, {ICON_SIZE, ICON_SIZE}, *this->_icon);
		// -------
	}
	// -------
} // namespace rawrbox
