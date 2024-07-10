#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/progress_bar.hpp>

namespace rawrbox {
	// UTILS ----
	UIProgressBar::UIProgressBar(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {
		this->_overlay = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/ui/overlay/overlay.png")->get();
		this->_bg = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("./assets/textures/ui/background_grid.png")->get();
	}

	UIProgressBar::~UIProgressBar() {
		this->_overlay = nullptr;
		this->_bg = nullptr;
	}

	void UIProgressBar::showPercent(bool show) { this->_percent = show; }
	bool UIProgressBar::isPercentVisible() const { return this->_percent; }

	void UIProgressBar::setBarColor(const rawrbox::Color& color) { this->_progressColor = color; }
	const rawrbox::Color& UIProgressBar::getBarColor() const { return this->_progressColor; }

	float UIProgressBar::getValue() const { return this->_value; }
	void UIProgressBar::setValue(float value) {
		this->_value = std::clamp(value, 0.F, 100.F);
		this->onValueChange(value);
	}
	// ---------

	// DRAWING ---
	void UIProgressBar::draw(rawrbox::Stencil& stencil) {
		const auto& size = this->getSize();

		// BG
		stencil.drawTexture({0, 0}, {size.x, size.y}, *this->_bg, Color::RGBAHex(0xFFFFFFA4), {}, {static_cast<float>(size.x) / static_cast<float>(this->_bg->getSize().x), static_cast<float>(size.y) / static_cast<float>(this->_bg->getSize().y)});
		// ------

		// BAR
		float max = size.x;
		float val = ((this->_value * static_cast<float>(max)) / 100.F);
		if (val <= 0) val = 0;

		stencil.drawBox({0, 0}, {val, size.y}, this->_progressColor);
		stencil.drawLine({0, size.y - 1}, {size.x, size.y - 1}, Color::RGBAHex(0x0000004A));
		// --------------------

		if (rawrbox::DEBUG_FONT_REGULAR != nullptr && this->_percent) {
			auto color = this->_progressColor * 0.15F;
			color.a = 1.F;

			stencil.drawText(*rawrbox::DEBUG_FONT_REGULAR, std::to_string(static_cast<int>(this->_value)), {5, size.y / 2.F}, color, rawrbox::Alignment::Left, rawrbox::Alignment::Center);
		}

		// BORDER--
		stencil.pushOutline({1});
		stencil.drawBox({0, 0}, {size.x, size.y}, rawrbox::Colors::Black());
		stencil.popOutline();
		//----
	}

	// FOCUS HANDLE ---
	bool UIProgressBar::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; }
	// -----

	void UIProgressBar::afterDraw(Stencil& stencil) {
		if (this->_overlay == nullptr) return;

		const auto& size = this->getSize();
		stencil.drawTexture({}, size, *this->_overlay, Color::RGBAHex(0xFFFFFF0A), {}, {static_cast<float>(size.x) / static_cast<float>(this->_overlay->getSize().x / 2), static_cast<float>(size.y) / static_cast<float>(this->_overlay->getSize().y / 2)});
	}
	// -----
} // namespace rawrbox
