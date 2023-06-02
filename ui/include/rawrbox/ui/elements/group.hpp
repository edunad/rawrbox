
#pragma once
#include <rawrbox/ui/base.hpp>

namespace rawrbox {
	class UIGroup : public rawrbox::UIBase {
	protected:
		float _border = 0.F;

	public:
		// UTILS ----
		virtual void setBorder(float border);
		[[nodiscard]] virtual float getBorder() const;

		virtual void sizeToContents();
		// ---------

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox