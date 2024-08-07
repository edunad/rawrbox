#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/ui/container.hpp>
#include <rawrbox/utils/event.hpp>

#include <filesystem>

namespace rawrbox {
	class TextureBase;
	class Font;

	class UIButton : public rawrbox::UIContainer {
		rawrbox::Color _backgroundColor = rawrbox::Color::RGBHex(0x36393f);
		rawrbox::Color _textColor = rawrbox::Color::RGBHex(0xf5f6fa);
		rawrbox::Color _textureColor = rawrbox::Colors::White();
		rawrbox::Color _borderColor = rawrbox::Color::RGBAHex(0x0000004A);

		std::string _text;
		rawrbox::Vector2 _textureSize = {12, 12};

		bool _border = true;
		bool _enabled = false;

		std::string _tooltip;

		// RESOURCES ---
		rawrbox::TextureBase* _texture = nullptr;
		rawrbox::TextureBase* _overlay = nullptr;
		rawrbox::Font* _regular = nullptr;
		// -----------------

	public:
		rawrbox::Event<> onClick;

		UIButton(rawrbox::UIRoot* root);
		UIButton(const UIButton&) = default;
		UIButton(UIButton&&) = delete;
		UIButton& operator=(const UIButton&) = default;
		UIButton& operator=(UIButton&&) = delete;
		~UIButton() override {
			this->_texture = nullptr;
			this->_overlay = nullptr;
			this->_regular = nullptr;
		}

		// UTILS -----
		virtual void setTextureSize(const rawrbox::Vector2& size);

		virtual void setTextureColor(const rawrbox::Color& color);
		[[nodiscard]] virtual const rawrbox::Color& getTextureColor() const;

		virtual void setTextColor(const rawrbox::Color& color);
		[[nodiscard]] virtual const rawrbox::Color& getTextColor() const;

		virtual void setBackgroundColor(const rawrbox::Color& color);
		[[nodiscard]] virtual const rawrbox::Color& getBackgroundColor() const;

		virtual void setBorderColor(const rawrbox::Color& color);
		[[nodiscard]] virtual const rawrbox::Color& getBorderColor() const;

		virtual void setText(const std::string& text, uint16_t size = 16);
		[[nodiscard]] virtual const std::string& getText() const;

		virtual void setTooltip(const std::string& text);
		[[nodiscard]] virtual const std::string& getTooltip() const;

		virtual void setTexture(rawrbox::TextureBase* texture);
		virtual void setTexture(const std::filesystem::path& path);

		virtual void setEnabled(bool enabled);
		[[nodiscard]] bool isEnabled() const;

		virtual void setBorder(bool enabled);
		[[nodiscard]] virtual bool borderEnabled() const;
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
