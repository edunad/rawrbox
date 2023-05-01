#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include <map>
#include <utility>
#include <vector>

namespace rawrBox {

	class TextEngine { // Not threadsafe, make one engine per thread
		static uint32_t atlasID;

		std::vector<std::unique_ptr<rawrBox::Font>> _fonts;
		std::map<uint32_t, std::unique_ptr<rawrBox::TextureAtlas>> _atlas;
		void initialize();
		void destroy();

	public:
		FT_Library ft = nullptr;

		TextEngine();
		TextEngine(const TextEngine& other) = delete;
		TextEngine(TextEngine&& other) = delete;
		~TextEngine();

		TextEngine& operator=(const TextEngine& other) = delete;
		TextEngine& operator=(TextEngine&& other) = delete;

		std::pair<uint32_t, rawrBox::TextureAtlas*> requestAtlas(int width, int height);
		rawrBox::TextureAtlas* getAtlas(uint32_t id);

		rawrBox::Font& load(std::string filename, uint32_t size);
	};
} // namespace rawrBox