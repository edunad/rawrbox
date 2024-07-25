#include <rawrbox/render/stencil.hpp>
#include <rawrbox/ui/elements/loading.hpp>

namespace rawrbox {
	UILoading::UILoading(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {}

	// FOCUS ---
	bool UILoading::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; };
	// ---

	// DRAW ----
	void UILoading::draw(rawrbox::Stencil& stencil) {
		const auto& size = this->getContentSize();
		stencil.drawLoading({}, size, rawrbox::Color::RGBAHex(0xFFFFFF5A));
	}
	// -------
} // namespace rawrbox
