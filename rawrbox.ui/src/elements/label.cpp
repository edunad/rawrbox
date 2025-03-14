
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/text/font.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/label.hpp>

namespace rawrbox {
	UILabel::UILabel(rawrbox::UIRoot* root) : rawrbox::UIContainer(root), _font(rawrbox::DEBUG_FONT_REGULAR) {}
	UILabel::~UILabel() {
		this->_font = nullptr;
	}

	// FOCUS HANDLE ---
	bool UILabel::hitTest(const rawrbox::Vector2f& /*point*/) const { return false; }
	// -----

	void UILabel::setPos(const rawrbox::Vector2f& pos) {
		rawrbox::UIContainer::setPos({std::roundf(pos.x), std::roundf(pos.y)});
	}

	// UTILS ----
	void UILabel::setColor(const rawrbox::Color& col) { this->_color = col; }
	const rawrbox::Color& UILabel::getColor() const { return this->_color; }

	void UILabel::setShadowPos(const rawrbox::Vector2f& pos) { this->_shadow = pos; }
	const rawrbox::Vector2f& UILabel::getShadowPos() const { return this->_shadow; }

	void UILabel::setShadowColor(const rawrbox::Color& col) { this->_shadowColor = col; }
	const rawrbox::Color& UILabel::getShadowColor() const { return this->_shadowColor; }

	void UILabel::setText(const std::string& text) { this->_text = text; }
	const std::string& UILabel::getText() const { return this->_text; }

	void UILabel::setFont(const std::filesystem::path& font, uint16_t size) {
		this->_font = rawrbox::RESOURCES::getFile<rawrbox::ResourceFont>(font)->getSize(size);
	}

	void UILabel::setFont(rawrbox::Font* font) {
		if (font == nullptr) throw std::runtime_error("[RawrBox-UI] Invalid font");
		this->_font = font;
	}

	rawrbox::Font* UILabel::getFont() const { return this->_font; }

	void UILabel::sizeToContents() {
		if (this->_font == nullptr) return;
		this->setSize(this->_font->getStringSize(this->_text));
	}
	// ----------

	// DRAW ----
	void UILabel::draw(Stencil& stencil) {
		if (this->_font == nullptr) return;

		stencil.drawText(*this->_font, this->_text, this->_shadow, this->_shadowColor);
		stencil.drawText(*this->_font, this->_text, {0, 0}, this->_color);
	}
	// ----------
} // namespace rawrbox
