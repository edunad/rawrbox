
#pragma once
#include <rawrbox/ui/container.hpp>

namespace rawrbox {
	class UIGroup : public rawrbox::UIContainer {
	protected:
		float _border = 0.F;

	public:
		UIGroup(rawrbox::UIRoot* root);
		~UIGroup() override = default;
		UIGroup(const UIGroup&) = default;
		UIGroup(UIGroup&&) = delete;
		UIGroup& operator=(const UIGroup&) = default;
		UIGroup& operator=(UIGroup&&) = delete;

		// UTILS ----
		virtual void setBorder(float border);
		[[nodiscard]] virtual float getBorder() const;

		virtual void sizeToContents();
		// ---------

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
