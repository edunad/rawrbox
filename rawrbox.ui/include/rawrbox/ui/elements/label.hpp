#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/ui/container.hpp>

#include <string>

namespace rawrbox {
	class UILabel : public rawrbox::UIContainer {
		rawrbox::Color _color = rawrbox::Colors::White();
		rawrbox::Color _shadowColor = rawrbox::Colors::Transparent();

		// RESOURCES ---
		rawrbox::Font* _font = nullptr;
		// -----------------

		std::string _text = "";
		rawrbox::Vector2f _shadow = {1, 1};

	public:
		UILabel() = default;
		UILabel(const UILabel&) = default;
		UILabel(UILabel&&) = delete;
		UILabel& operator=(const UILabel&) = default;
		UILabel& operator=(UILabel&&) = delete;
		~UILabel() override {
			this->_font = nullptr;
		}

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		void initialize() override;

		// UTILS ----
		virtual void setColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getColor() const;

		virtual void setShadowPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] virtual const rawrbox::Vector2f& getShadowPos() const;

		virtual void setShadowColor(const rawrbox::Color& col);
		[[nodiscard]] virtual const rawrbox::Color& getShadowColor() const;

		virtual void setText(const std::string& text);
		[[nodiscard]] virtual const std::string& getText() const;

		virtual void setFont(rawrbox::Font* font);
		virtual void setFont(const std::filesystem::path& font, uint16_t size = 11);
		[[nodiscard]] virtual rawrbox::Font* getFont() const;

		virtual void sizeToContents();
		// -----

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
