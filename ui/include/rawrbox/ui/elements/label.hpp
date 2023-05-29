#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/render/resources/font.hpp>
#include <rawrbox/ui/base.hpp>

#include <string>

namespace rawrbox {
	class UILabel : public rawrbox::UIBase {
		rawrbox::Color _color = rawrbox::Colors::White;
		rawrbox::Color _shadowColor = rawrbox::Colors::Transparent;

		// RESOURCES ---
		std::weak_ptr<rawrbox::Font> _font;
		// -----------------

		std::string _text = "";
		rawrbox::Vector2f _shadow = {1, 1};

	public:
		~UILabel() override;

		// FOCUS HANDLE ---
		[[nodiscard]] bool hitTest(const rawrbox::Vector2f& point) const override;
		// -----

		// UTILS ----
		void setColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getColor() const;

		void setShadowPos(const rawrbox::Vector2f& pos);
		[[nodiscard]] const rawrbox::Vector2f& getShadowPos() const;

		void setShadowColor(const rawrbox::Color& col);
		[[nodiscard]] const rawrbox::Color& getShadowColor() const;

		void setText(const std::string& text);
		[[nodiscard]] const std::string& getText() const;

		void setFont(std::shared_ptr<rawrbox::Font> font);
		void setFont(const std::filesystem::path& font, int size = 11);
		[[nodiscard]] std::weak_ptr<rawrbox::Font> getFont() const;

		void sizeToContents();
		// -----

		// DRAW ----
		void draw(rawrbox::Stencil& stencil) override;
		// -------
	};
} // namespace rawrbox
