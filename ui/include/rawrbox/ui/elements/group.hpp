
#pragma once
#include <rawrbox/ui/container.hpp>

namespace rawrbox {
	class UIGroup : public rawrbox::UIContainer {
	protected:
		float _border = 0.F;

#ifdef RAWRBOX_SCRIPTING
		void initializeLua(rawrbox::Mod* mod) override;
#endif
	public:
		~UIGroup() override = default;
		UIGroup() = default;
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
