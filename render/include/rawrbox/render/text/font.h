#pragma once

#include <rawrBox/render/texture/atlas.h>
#include <rawrBox/math/vector2.hpp>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <string>
#include <vector>
#include <memory>

namespace rawrBox {
	struct Glyph {
		FT_ULong codepoint;
		FT_UInt glyphIndex;

		rawrBox::Vector2f bearing;
		rawrBox::Vector2f advance;
		rawrBox::Vector2f textureTopLeft;
		rawrBox::Vector2f textureBottomRight;
		rawrBox::Vector2i size;
	};

	namespace TextFlags {
		const uint32_t NONE = 0;
		const uint32_t STROKE = 1 << 1;
	};

	class Font {

		private:
			std::string _file;
			uint32_t _size;

			// Freetype
			FT_Stroker _stroker;
			// -------------

			std::vector<Glyph> _glyphs;

			// Loading ---
			uint32_t _loadFlags = TextFlags::NONE;
			bool undead = false;
			// -----

			// SIZE ----
			uint32_t bitmapR = -1;
			uint32_t bitmapW = -1;
			FT_Int bitmapX = -1;
			FT_Int bitmapY = -1;
			// -----

			// GLYPH LOADING -----
			Glyph loadGlyph(FT_ULong character);
			std::vector<unsigned char> generateGlyphStroke();
			std::vector<unsigned char> generateGlyph(std::vector<unsigned char>& buffer);

			void preloadGlyphs(std::string chars);
			// -----
		public:
			FT_Face face;
			std::unique_ptr<rawrBox::TextureAtlas> atlas;

			Font(FT_Library& ft, std::string filename, uint32_t size, uint32_t flags = TextFlags::NONE);
			~Font();

			// UTILS --
			float getLineHeight() const;
			bool hasGlyph(uint32_t codepoint) const;
			const Glyph& getGlyph(uint32_t codepoint) const;
			float getKerning(const Glyph& left, const Glyph& right) const;
			rawrBox::Vector2 getStringSize(const std::string& text) const;
			// ----
	};

}
