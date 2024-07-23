
#pragma once
#include <rawrbox/ui/container.hpp>

namespace rawrbox {
	class UILoading : public rawrbox::UIContainer {
	public:
		UILoading(rawrbox::UIRoot* root);
		~UILoading() override = default;
		UILoading(const UILoading&) = default;
		UILoading(UILoading&&) = delete;
		UILoading& operator=(const UILoading&) = default;
		UILoading& operator=(UILoading&&) = delete;

		// FOCUS ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// ---

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
