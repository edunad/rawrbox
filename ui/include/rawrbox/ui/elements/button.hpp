#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <string>

namespace rawrbox {
	class UIButton : public rawrbox::UIContainer {
		rawrbox::Color _backgroundColor = rawrbox::Color::RGBHex(0x36393f);
		rawrbox::Color _textColor = rawrbox::Color::RGBHex(0xf5f6fa);
		rawrbox::Color _textureColor = rawrbox::Colors::White;

		std::string _text;
		rawrbox::Vector2 _textureSize = {12, 12};

		bool _border = true;
		bool _enabled = false;

		std::string _tooltip = "";

		// RESOURCES ---
		rawrbox::TextureBase* _texture = nullptr;
		rawrbox::TextureImage* _overlay = nullptr;
		rawrbox::Font* _consola = nullptr;
		// -----------------

	public:
		rawrbox::Event<> onClick;
		void initialize() override;

		// UTILS -----
		void setTextureSize(const rawrbox::Vector2& size);
		void setTextureColor(const rawrbox::Color& color);
		[[nodiscard]] const rawrbox::Color& getTextureColor() const;

		void setTextColor(const rawrbox::Color& color);
		[[nodiscard]] const rawrbox::Color& getTextColor() const;

		void setBackgroundColor(const rawrbox::Color& color);
		[[nodiscard]] const rawrbox::Color& getBackgroundColor() const;

		void setText(const std::string& text, int size = 16);
		[[nodiscard]] const std::string& getText() const;

		void setTooltip(const std::string& text);
		[[nodiscard]] const std::string& getTooltip() const;

		void setTexture(rawrbox::TextureBase& texture);
		void setTexture(const std::string& path);

		void setEnabled(bool enabled);
		[[nodiscard]] const bool isEnabled() const;

		virtual void setBorder(bool enabled);
		[[nodiscard]] virtual const bool borderEnabled() const;
		// -------

		// INPUTS ---
		void mouseUp(const rawrbox::Vector2i& mousePos, uint32_t button, uint32_t mods) override;
		// -----

		void setHovering(bool hovering) override;

		// DRAWING ---
		void draw(rawrbox::Stencil& stencil) override;
		void afterDraw(rawrbox::Stencil& stencil) override;
		// -----
	};
} // namespace rawrbox
