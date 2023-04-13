#pragma once

#include <rawrbox/render/text/font.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include <vector>

namespace rawrBox {
	class TextEngine { // Not threadsafe, make one engine per thread
		FT_Library ft = nullptr;
		std::vector<std::unique_ptr<rawrBox::Font>> _fonts;

		void initialize();
		void destroy();
	public:
		TextEngine();
		TextEngine(const TextEngine& other) = delete;
		TextEngine(TextEngine&& other) = delete;
		~TextEngine();

		TextEngine& operator=(const TextEngine& other) = delete;
		TextEngine& operator=(TextEngine&& other) = delete;

		rawrBox::Font& load(std::string filename, uint32_t size, uint32_t flags = TextFlags::NONE);
	};
}
