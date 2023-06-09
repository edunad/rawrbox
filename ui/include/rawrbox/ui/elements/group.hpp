
#pragma once
#include <rawrbox/ui/container.hpp>

namespace rawrbox {
	class UIGroup : public rawrbox::UIContainer {
	protected:
		float _border = 0.F;

	public:
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
