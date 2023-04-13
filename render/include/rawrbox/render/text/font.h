#pragma once

#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_STROKER_H

#include <string>
#include <vector>
#include <memory>

namespace rawrBox {
	struct Glyph {
		uint32_t atlasID;

		FT_ULong codepoint;
		FT_UInt glyphIndex;

		rawrBox::Vector2f bearing;
		rawrBox::Vector2f advance;
		rawrBox::Vector2f textureTopLeft;
		rawrBox::Vector2f textureBottomRight;
		rawrBox::Vector2i size;
	};

	class TextEngine;
	class Font {

		private:
			TextEngine* _engine;
			std::vector<Glyph> _glyphs;

			std::string _file;
			uint32_t _size;

			// SIZE ----
			uint32_t bitmapR = -1;
			uint32_t bitmapW = -1;
			FT_Int bitmapX = -1;
			FT_Int bitmapY = -1;
			// -----

			// GLYPH LOADING -----
			Glyph loadGlyph(FT_ULong character);
			std::vector<unsigned char> generateGlyph();

			void preloadGlyphs(std::string chars);
			// -----
		public:
			FT_Face face;

			Font(TextEngine* engine, std::string filename, uint32_t size);
			~Font();

			// UTILS --
			float getLineHeight() const;
			bool hasGlyph(uint32_t codepoint) const;
			const Glyph& getGlyph(uint32_t codepoint) const;
			float getKerning(const Glyph& left, const Glyph& right) const;
			rawrBox::Vector2 getStringSize(const std::string& text) const;

			bgfx::TextureHandle& getHandle(const Glyph& g);
			// ----
	};

}
