#pragma once

#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/ui/elements/group.hpp>

#include <utility>

namespace rawrbox {
	struct UITab {
	public:
		std::string name;
		std::string id;

		rawrbox::UIGroup* group = nullptr;
		rawrbox::UIButton* button = nullptr;

		UITab(std::string id, std::string name, rawrbox::UIGroup* groupUI) : name(std::move(name)), id(std::move(id)), group(groupUI) {}
	};

	class UITabs : public rawrbox::UIContainer {

	protected:
		std::vector<rawrbox::UITab> _tabs = {};

		rawrbox::UITab* _activeTab = nullptr;
		rawrbox::UIGroup* _buttonGroup = nullptr;

		virtual void generate();

	public:
		UITabs(rawrbox::UIRoot* root, const std::vector<rawrbox::UITab>& tabs);
		UITabs(const UITabs&) = default;
		UITabs(UITabs&&) = delete;
		UITabs& operator=(const UITabs&) = default;
		UITabs& operator=(UITabs&&) = delete;
		~UITabs() override = default;

		void setSize(const rawrbox::Vector2f& size) override;

		// TABS ----
		virtual void setActive(size_t index);
		virtual void setEnabled(size_t index, bool enabled);

		[[nodiscard]] virtual float getTabHeight() const;
		[[nodiscard]] virtual float getButtonWidth() const;
		[[nodiscard]] virtual uint16_t getButtonFontSize() const;
		// ---------

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
