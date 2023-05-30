
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/label.hpp>

namespace rawrbox {
	UILabel::~UILabel() {
		this->_font.reset();
	}

	// FOCUS HANDLE ---
	bool UILabel::hitTest(const rawrbox::Vector2f& point) const { return false; }
	// -----

	// UTILS ----
	void UILabel::setColor(const rawrbox::Color& col) { this->_color = col; }
	const rawrbox::Color& UILabel::getColor() const { return this->_color; }

	void UILabel::setShadowPos(const rawrbox::Vector2f& pos) { this->_shadow = pos; }
	const rawrbox::Vector2f& UILabel::getShadowPos() const { return this->_shadow; }

	void UILabel::setShadowColor(const rawrbox::Color& col) { this->_shadowColor = col; }
	const rawrbox::Color& UILabel::getShadowColor() const { return this->_shadowColor; }

	void UILabel::setText(const std::string& text) { this->_text = text; }
	const std::string& UILabel::getText() const { return this->_text; }

	void UILabel::setFont(const std::filesystem::path& font, int size) {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(font)->getSize(size);
	}

	void UILabel::setFont(std::shared_ptr<rawrbox::Font> font) {
		if (font == nullptr) throw std::runtime_error("[RawrBox-UI] Invalid font");
		this->_font = font;
	}

	std::weak_ptr<rawrbox::Font> UILabel::getFont() const { return this->_font; }

	void UILabel::sizeToContents() {
		if (this->_font.expired()) return;
		this->setSize(this->_font.lock()->getStringSize(this->_text));
	}
	// ----------

	// DRAW ----
	void UILabel::draw(Stencil& stencil) {
		if (this->_font.expired()) return;

		auto fnt = this->_font.lock();
		stencil.drawText(fnt, this->_text, this->_shadow, this->_shadowColor);
		stencil.drawText(fnt, this->_text, {0, 0}, this->_color);
	}
	// ----------
} // namespace rawrbox
