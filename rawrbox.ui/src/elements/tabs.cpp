#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/group.hpp>
#include <rawrbox/ui/elements/tabs.hpp>
#include <rawrbox/utils/logger.hpp>

namespace rawrbox {
	// PRIVATE ----
	void UITabs::generate() {
		if (this->_buttonGroup == nullptr) throw rawrbox::Logger::err("UITabs", "Button group is null");
		const auto& size = this->getSize();

		const float height = this->getTabHeight();
		const float buttonWidth = this->getButtonWidth();

		// Create the main group ---
		this->_buttonGroup->setPos({0, 0});
		this->_buttonGroup->setSize({size.x, height});
		// ------

		for (size_t i = 0; i < this->_tabs.size(); i++) {
			auto& tab = this->_tabs[i];

			tab.group->setPos({0, height});
			tab.group->setSize({size.x, size.y - height});
			tab.group->setVisible(false);

			// Generate buttons ---
			tab.button = this->_buttonGroup->createChild<rawrbox::UIButton>();
			tab.button->setText(tab.name, this->getButtonFontSize());
			tab.button->setPos({static_cast<float>(i) * (buttonWidth + 2.F), 0});
			tab.button->setSize({buttonWidth, height});
			tab.button->setEnabled(true);
			tab.button->setTextColor(rawrbox::Colors::White());
			tab.button->setBorder(false);
			tab.button->setBackgroundColor(rawrbox::Color::RGBHex(0x282a2e));
			tab.button->onClick += [this, i]() {
				this->setActive(i);
			};
			// ------------
		}
	}

	void UITabs::updateTabs() {
		const auto& size = this->getSize();

		const float height = this->getTabHeight();
		const float buttonWidth = this->getButtonWidth();

		this->_buttonGroup->setSize({size.x, height});

		for (auto& tab : this->_tabs) {
			if (tab.button == nullptr || tab.group == nullptr) continue;

			tab.group->setSize({size.x, size.y - height});
			tab.button->setSize({buttonWidth, height});
		}
	}
	// -------------

	UITabs::UITabs(rawrbox::UIRoot* root, const std::vector<rawrbox::UITab>& tabs) : rawrbox::UIContainer(root), _tabs(tabs), _buttonGroup(this->createChild<rawrbox::UIGroup>()) {
		this->generate();
	}

	void UITabs::setSize(const rawrbox::Vector2f& size) {
		rawrbox::UIContainer::setSize(size);
		this->updateTabs();
	}

	// TABS ---
	void UITabs::setActive(size_t index) {
		if (index > this->_tabs.size()) throw rawrbox::Logger::err("UITabs", "Invalid index {}", index);

		auto& tab = this->_tabs[index];
		if (tab.button == nullptr) throw rawrbox::Logger::err("UITabs", "Invalid tab button from index {}", index);
		if (tab.group == nullptr) throw rawrbox::Logger::err("UITabs", "Invalid tab group from index {}", index);

		if (this->_activeTab == &tab) return;

		if (this->_activeTab != nullptr) {
			if (this->_activeTab->button != nullptr && this->_activeTab->group != nullptr) {
				this->_activeTab->button->setBackgroundColor(Color::RGBHex(0x282a2e));
				this->_activeTab->group->setVisible(false);
			}
		}

		tab.button->setBackgroundColor(Color::RGBHex(0x131418));
		tab.group->setVisible(true);

		this->_activeTab = &tab;
		onTabChange(tab.id);
	}

	void UITabs::setEnabled(size_t index, bool enabled) {
		if (index > this->_tabs.size()) throw rawrbox::Logger::err("UITabs", "Invalid index {}", index);

		auto& tab = this->_tabs[index];
		if (tab.button == nullptr) throw rawrbox::Logger::err("UITabs", "Invalid tab button from index {}", index);
		if (tab.group == nullptr) throw rawrbox::Logger::err("UITabs", "Invalid tab group from index {}", index);

		tab.button->setEnabled(enabled);
	}

	float UITabs::getTabHeight() const { return 20.F; }
	float UITabs::getButtonWidth() const { return 70.F; }

	uint16_t UITabs::getButtonFontSize() const { return 11U; }
	// ----

	// DRAW ----
	void UITabs::draw(rawrbox::Stencil& stencil) {
		const auto& size = this->getSize();
		const auto height = this->getTabHeight();

		// Top bar
		stencil.drawBox({0, 0}, {size.x, height}, rawrbox::Color::RGBHex(0x202226));
		stencil.drawBox({0, height}, {size.x, 1}, rawrbox::Color::RGBAHex(0x0000005A)); // line split
												// ----
	}
	// -------
} // namespace rawrbox
