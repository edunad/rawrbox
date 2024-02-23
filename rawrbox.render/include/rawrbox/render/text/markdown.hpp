#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.hpp>
#include <rawrbox/render/text/font.hpp>

#include <random>
#include <string>

namespace rawrbox {
	enum class MarkdownType {
		none,
		corrupt,
		text,
		linebreak,
	};

	enum class MarkdownOffset {
		left,
		center,
		right
	};

	struct MarkdownElement {
		MarkdownType type = MarkdownType::none;
		MarkdownOffset offset = MarkdownOffset::left;

		std::string text;

		rawrbox::Vector2f size = {};

		rawrbox::Font* font = nullptr;

		rawrbox::Colorf defaultColor = {};
		rawrbox::Colorf color = {};
	};

	class Markdown {
	protected:
		std::random_device _prng;

		rawrbox::MarkdownElement _currentStyle;
		std::vector<rawrbox::MarkdownElement> _elements;

		void parseChunk(const std::string& text);
		void pushTextSoFar(const std::string& text, size_t& textStart, size_t endPos);

	public:
		rawrbox::Font* fontItalic = nullptr;
		rawrbox::Font* fontBold = nullptr;
		rawrbox::Font* fontRegular = nullptr;

		void clear();
		void regenFonts();
		void parse(const std::string& text, rawrbox::Colorf defaultColor = rawrbox::Colors::White());
		void render(rawrbox::Stencil& stencil, rawrbox::Vector2f pos);

		[[nodiscard]] rawrbox::Vector2f getSize() const;
	};
} // namespace rawrbox
