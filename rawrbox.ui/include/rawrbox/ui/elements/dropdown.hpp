
#pragma once
#include <rawrbox/ui/container.hpp>
#include <rawrbox/ui/elements/virtual_list.hpp>

namespace rawrbox {
	class TextureBase;
	class UIDropdown : public rawrbox::UIContainer {
	protected:
		rawrbox::TextureBase* _icon = nullptr;

		// OPTIONS ---
		rawrbox::UIVirtualList<std::string>* _list = nullptr;
		rawrbox::Vector2f _originalSize = {};

		size_t _selected = 0;
		// -------------

		void calculateListSize();

	public:
		constexpr static float ICON_SIZE = 16.F;
		rawrbox::Event<size_t, const std::string&> onSelectionChange;

		UIDropdown(rawrbox::UIRoot* root, const std::vector<std::string>& options = {});
		UIDropdown(const UIDropdown&) = default;
		UIDropdown(UIDropdown&&) = delete;
		UIDropdown& operator=(const UIDropdown&) = default;
		UIDropdown& operator=(UIDropdown&&) = delete;
		~UIDropdown() override = default;

		// UTILS ----
		void setOptions(const std::vector<std::string>& options);
		void addOption(const std::string& option);
		void removeOption(size_t index);

		void setActive(size_t option);
		[[nodiscard]] size_t getSelected() const;
		[[nodiscard]] const std::string& getSelectedValue() const;
		// ---------

		// OVERRIDE ---
		void setSize(const rawrbox::Vector2f& size) override;
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;

		void setFocused(bool focused) override;
		// ----------

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
