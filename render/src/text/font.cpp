#include <rawrbox/render/text/font.h>

#include <fmt/format.h>

#include <utf8.h>
#include <iostream>

namespace rawrBox {
	Font::~Font() {
		if(undead) return;

		if (FT_Done_Face(this->face) != 0) fmt::print(stderr, "Error: failed to clean up font\n");
		if((this->_loadFlags & TextFlags::STROKE) > 0) FT_Stroker_Done(this->_stroker);
	}

	Font::Font(FT_Library& ft, std::string _filename, uint32_t size, uint32_t _flags) : _file(_filename), _size(size), _loadFlags(_flags) {
		if (FT_New_Face(ft, this->_file.c_str(), 0, &this->face) != FT_Err_Ok) {
			throw std::runtime_error(fmt::format("Error: failed to load font: {}", this->_file));
		}

		if((this->_loadFlags & TextFlags::STROKE) > 0) {
			if(FT_Stroker_New(ft, &this->_stroker) != FT_Err_Ok) {
				throw std::runtime_error(fmt::format("Error: failed to initialize font stroker: {}", this->_file));
			}
		}

		FT_Set_Char_Size(this->face, 0, this->_size * 64, 72, 72); // DPI = 72
		FT_Select_Charmap(this->face, FT_ENCODING_UNICODE);

		// Default chars
		std::string preload = "~!@#$%^&*()_+`1234567890-=QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm|<>?,./:;\"'}{][ \\";
		this->atlas = std::make_unique<rawrBox::TextureAtlas>(512, bgfx::TextureFormat::RG8);
		this->atlas->upload(); // Move it to initialize?

		this->preloadGlyphs(preload);
	}

	void Font::preloadGlyphs(std::string chars) {
		auto charsIter = chars.begin();
		while(charsIter < chars.end()) {
			this->loadGlyph(utf8::next(charsIter, chars.end()));
		}
	}

	// UTILS --
	float Font::getLineHeight() const {
		return static_cast<float>(this->face->size->metrics.height >> 6);
	}

	bool Font::hasGlyph(uint32_t codepoint) const {
		return std::find_if(this->_glyphs.begin(), this->_glyphs.end(), [codepoint](auto glyph) {
			return glyph.codepoint == codepoint;
		}) != this->_glyphs.end();
	}

	const Glyph& Font::getGlyph(uint32_t codepoint) const {
		return *std::find_if(this->_glyphs.begin(), this->_glyphs.end(), [codepoint](auto glyph) { return glyph.codepoint == codepoint; });
	}

	float Font::getKerning(const Glyph& left, const Glyph& right) const {
		FT_Vector kerning;

		FT_Get_Kerning(this->face, left.glyphIndex, right.glyphIndex, FT_KERNING_UNFITTED, &kerning);
		return static_cast<float>(kerning.x >> 6);
	}

	rawrBox::Vector2 Font::getStringSize(const std::string& text) const {
		const float lineheight = this->getLineHeight();
		if (text.empty()) return {0, lineheight};

		rawrBox::Vector2 total;
		rawrBox::Vector2 pos;

		const Glyph* prevGlyph = nullptr;

		uint32_t point = 0;
		auto beginIter = text.begin();
		auto endIter = utf8::find_invalid(text.begin(), text.end());

		while (beginIter != endIter) {
			point = utf8::next(beginIter, endIter);
			if (point == '\n') {
				pos.y += lineheight;
				pos.x = 0;
				prevGlyph = nullptr;
				continue;
			}

			if (!hasGlyph(point)) continue;

			const auto& glyph = getGlyph(point);
			auto maxh = std::max(static_cast<float>(glyph.size.y), lineheight);

			if (prevGlyph != nullptr) pos.x += getKerning(glyph, *prevGlyph);
			pos.x += glyph.advance.x;

			if (pos.x > total.x) total.x = pos.x;
			if (pos.y + maxh > total.y) total.y = pos.y + maxh;

			prevGlyph = &glyph;
		}

		return total;
	}
	// -------

	// GLYPH LOADING -----
	Glyph Font::loadGlyph(FT_ULong character) {
		if (this->hasGlyph(character)) return getGlyph(character);

		FT_UInt charIndx = FT_Get_Char_Index(this->face, character);
		if (charIndx == 0) return {};

		if (FT_Load_Glyph(this->face, charIndx, FT_LOAD_NO_BITMAP) != FT_Err_Ok) return {};

		std::vector<unsigned char> buffer = this->generateGlyphStroke();
		buffer = this->generateGlyph(buffer);

		auto& atlasNode = this->atlas->addSprite(bitmapW, bitmapR, buffer);

		Glyph glyph {
			character,
			charIndx,
			{
				static_cast<float>(this->face->glyph->metrics.horiBearingX) / 64.f,
				static_cast<float>(this->face->glyph->metrics.horiBearingY) / 64.f
			},
			{
				static_cast<float>(this->face->glyph->advance.x >> 6),
				static_cast<float>(this->face->glyph->advance.y >> 6)
			},
			{
				atlasNode.x / static_cast<float>(this->atlas->size),
				atlasNode.y / static_cast<float>(this->atlas->size)
			},
			{
				(atlasNode.x + atlasNode.width) / static_cast<float>(this->atlas->size),
				(atlasNode.y + atlasNode.height) / static_cast<float>(this->atlas->size)
			},
			{
				static_cast<float>(this->face->glyph->metrics.width) / 64.f,
				static_cast<float>(this->face->glyph->metrics.height) / 64.f
			},
		};

		this->_glyphs.push_back(glyph);
		return glyph;
	}

	std::vector<unsigned char> Font::generateGlyphStroke() {
		if((this->_loadFlags & TextFlags::STROKE) == 0) return {};

		FT_Glyph glyphDescStroke;
		if (FT_Get_Glyph(this->face->glyph, &glyphDescStroke) != FT_Err_Ok) return {};

		FT_Pos outlineThickness = 2;
		FT_Stroker_Set(this->_stroker, static_cast<FT_Fixed>(outlineThickness * static_cast<float>(1 << 6)), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0 );

		if(FT_Glyph_StrokeBorder(&glyphDescStroke, this->_stroker, false, true) != FT_Err_Ok) return {};
		if(FT_Glyph_To_Bitmap(&glyphDescStroke, FT_RENDER_MODE_SDF, nullptr, true) != FT_Err_Ok) return {};

		FT_BitmapGlyph glyph_bitmap = reinterpret_cast<FT_BitmapGlyph>(glyphDescStroke);
		FT_Bitmap *bitmap_stroke = &glyph_bitmap->bitmap;
		if(bitmap_stroke == nullptr) return {};

		bitmapW = bitmap_stroke->width;
		bitmapR = bitmap_stroke->rows;

		bitmapX = glyph_bitmap->left;
		bitmapY = glyph_bitmap->top;

		this->face->glyph->advance.x += outlineThickness;
		this->face->glyph->advance.y += outlineThickness;

		auto buffer = std::vector<unsigned char>(bitmapW * bitmapR * 2, 0); // * 2 -> 2 color channels (red and green)
		for ( unsigned int i = 0; i < bitmapW * bitmapR; ++ i) {
			buffer[i * 2 + 1] = bitmap_stroke->buffer[i]; // + 1 -> 2nd color channel
		}

		FT_Done_Glyph( glyphDescStroke ); // Release
		return buffer;
	}

	std::vector<unsigned char> Font::generateGlyph(std::vector<unsigned char>& buffer) {
		FT_Glyph glyphDescFill;
		if(FT_Get_Glyph(this->face->glyph, &glyphDescFill) != FT_Err_Ok) return {};
		if(FT_Glyph_To_Bitmap( &glyphDescFill, FT_RENDER_MODE_SDF, nullptr, true) != FT_Err_Ok) return {};

		FT_BitmapGlyph glyph_bitmap = reinterpret_cast<FT_BitmapGlyph>(glyphDescFill);
		FT_Bitmap *bitmap_fill = &glyph_bitmap->bitmap;
		if(bitmap_fill == nullptr) return {};

		if((this->_loadFlags & TextFlags::STROKE) == 0) {
			bitmapW = bitmap_fill->width;
			bitmapR = bitmap_fill->rows;

			buffer = std::vector<unsigned char>(bitmapW * bitmapR * 2, 0); // * 2 -> 2 color channels (red and green)
			for ( unsigned int i = 0; i < bitmapW * bitmapR; ++ i)
				buffer[i * 2] = bitmap_fill->buffer[i];      // + 0 -> 1st color channel

		} else {
			unsigned int cx_fill = bitmap_fill->width;
			unsigned int cy_fill = bitmap_fill->rows;

			unsigned int offset_x = (bitmapW - cx_fill) / 2; // offset because the bitmap my be smaller,
			unsigned int offset_y = (bitmapR - cy_fill) / 2; // then the former

			for(unsigned int y = 0; y < cy_fill; ++ y ) {
				for(unsigned int x = 0; x < cx_fill; ++ x ) {
					unsigned int i_source = y * cx_fill + x;
					unsigned int i_target = (y + offset_y) * bitmapW + x + offset_x;

					buffer[i_target * 2] = bitmap_fill->buffer[i_source]; // + 0 -> 1st color channel
				}
			}
		}

		FT_Done_Glyph( glyphDescFill ); // Release
		return buffer;
	}
	// --------
}
