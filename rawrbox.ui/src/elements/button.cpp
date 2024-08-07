
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/ui/elements/button.hpp>
#include <rawrbox/utils/keys.hpp>

namespace rawrbox {
	UIButton::UIButton(rawrbox::UIRoot* root) : rawrbox::UIContainer(root) {
		this->_overlay = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>("assets/textures/ui/overlay/overlay.png")->get();
	}

	// UTILS -----
	void UIButton::setTextureSize(const rawrbox::Vector2& size) { this->_textureSize = size; }
	void UIButton::setTextureColor(const rawrbox::Color& color) { this->_textureColor = color; }
	const rawrbox::Color& UIButton::getTextureColor() const { return this->_textureColor; }

	void UIButton::setBackgroundColor(const rawrbox::Color& color) { this->_backgroundColor = color; }
	const rawrbox::Color& UIButton::getBackgroundColor() const { return this->_backgroundColor; }

	void UIButton::setTextColor(const rawrbox::Color& color) { this->_textColor = color; }
	const rawrbox::Color& UIButton::getTextColor() const { return this->_textColor; }

	void UIButton::setBorderColor(const rawrbox::Color& color) { this->_borderColor = color; }
	const rawrbox::Color& UIButton::getBorderColor() const { return this->_borderColor; }

	void UIButton::setText(const std::string& text, uint16_t size) {
		this->_regular = rawrbox::DEBUG_FONT_REGULAR->scale(size);
		this->_text = text;
	}
	const std::string& UIButton::getText() const { return this->_text; }

	void UIButton::setTooltip(const std::string& text) { this->_tooltip = text; }
	const std::string& UIButton::getTooltip() const { return this->_tooltip; }

	void UIButton::setTexture(rawrbox::TextureBase* texture) { this->_texture = texture; }
	void UIButton::setTexture(const std::filesystem::path& path) {
		this->_texture = rawrbox::RESOURCES::getFile<rawrbox::ResourceTexture>(path)->get();
	}

	void UIButton::setEnabled(bool enabled) { this->_enabled = enabled; }
	bool UIButton::isEnabled() const { return this->_enabled; }

	void UIButton::setBorder(bool enabled) { this->_border = enabled; }
	bool UIButton::borderEnabled() const { return this->_border; }
	// ---------

	// INPUTS ---
	void UIButton::mouseUp(const rawrbox::Vector2i& /*mousePos*/, uint32_t button, uint32_t /*mods*/) {
		if (!this->_enabled || button != rawrbox::MOUSE_BUTTON_1) return;
		onClick();
	}
	// ---------

	void UIButton::setHovering(bool hover) {
		rawrbox::UIContainer::setHovering(hover);
		if (this->_tooltip.empty()) return;

		/*auto& game = IasGame::getInstance();
		auto pos = getPosAbsolute();
		auto& size = getSize();

		if (hover) {
			game.tooltip->setMaxSize({200, 12});
			game.tooltip->setPosition({pos.x + size.x / 2.f, pos.y + size.y + 5.f});

			game.tooltip->setTitle(tooltip);
			game.tooltip->setVisible(true);
		} else {
			game.tooltip->setVisible(false);
		}*/
	}

	// DRAWING ---
	void UIButton::draw(rawrbox::Stencil& stencil) {
		const auto& size = this->getContentSize();

		// Background ----
		stencil.drawBox({}, size, this->_backgroundColor); // Fake outline border
		if (this->_border) {
			stencil.pushOutline({2});
			stencil.drawBox({}, size, this->_borderColor); // Fake outline border
			stencil.popOutline();
		}
		// ---------------

		if (this->_texture != nullptr) {
			// No text, center icon
			if (this->_text.empty()) {
				stencil.drawTexture({(size.x - this->_textureSize.x) / 2, (size.y - this->_textureSize.y) / 2}, this->_textureSize, *this->_texture, this->_textureColor);
			} else {
				// Set icon on left
				stencil.drawTexture({2, (size.y - this->_textureSize.y) / 2}, this->_textureSize, *this->_texture, this->_textureColor); // Padding
			}
		}

		if (!this->_text.empty() && this->_regular != nullptr) {
			auto progSize = this->_regular->getStringSize(this->_text);

			// No icon, center text
			if (this->_texture == nullptr) {
				stencil.drawText(*this->_regular, this->_text, {(size.x - progSize.x) / 2, (size.y - progSize.y) / 2}, this->_textColor);
			} else {
				// Text after icon
				stencil.drawText(*this->_regular, this->_text, {this->_textureSize.x + 4, (size.y - progSize.y) / 2}, this->_textColor);
			}
		}

		// Overlay on top
		if (!this->_enabled) {
			stencil.drawBox({}, size, Color::RGBAHex(0x000000AF));
		} else if (this->hovering()) {
			stencil.drawBox({}, size, Color::RGBAHex(0x0000001A));
		}
	}

	void UIButton::afterDraw(rawrbox::Stencil& stencil) {
		if (this->_overlay == nullptr) return;

		const auto& size = this->getSize();
		stencil.drawTexture({}, size, *this->_overlay, Color::RGBAHex(0xFFFFFF0A), {}, {static_cast<float>(size.x) / static_cast<float>(this->_overlay->getSize().x / 2), static_cast<float>(size.y) / static_cast<float>(this->_overlay->getSize().y / 2)});
	}
	// ---------

} // namespace rawrbox
