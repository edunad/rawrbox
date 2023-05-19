#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H

#include <map>
#include <utility>

namespace rawrbox {

	class TextEngine {

		static std::map<std::string, std::shared_ptr<rawrbox::Font>> _fonts;
		static std::map<uint32_t, std::shared_ptr<rawrbox::TextureAtlas>> _atlas;

	public:
		static uint32_t atlasID;
		static FT_Library ft;

		static void initialize();
		static void shutdown();

		static std::pair<uint32_t, std::shared_ptr<rawrbox::TextureAtlas>> requestAtlas(int width, int height, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8);
		static std::shared_ptr<rawrbox::TextureAtlas> getAtlas(uint32_t id);

		static std::weak_ptr<rawrbox::Font> load(std::string filename, uint32_t size);
	};
} // namespace rawrbox
