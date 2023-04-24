#include <rawrbox/render/text/engine.h>
#include <rawrbox/render/text/font.h>

#include <fmt/format.h>
#include <utf8.h>

#include <iostream>
#include <utility>

namespace rawrBox {
	Font::~Font() {
		if (FT_Done_Face(this->face) != 0) fmt::print(stderr, "Error: failed to clean up font\n");
	}

	Font::Font(rawrBox::TextEngine* engine, std::string _filename, uint32_t size) : _engine(engine), _file(std::move(_filename)), _size(size) {
		if (FT_New_Face(engine->ft, this->_file.c_str(), 0, &this->face) != FT_Err_Ok) {
			throw std::runtime_error(fmt::format("Error: failed to load font: {}", this->_file));
		}

		FT_Set_Char_Size(this->face, 0, this->_size * 64, 72, 72); // DPI = 72
		FT_Select_Charmap(this->face, FT_ENCODING_UNICODE);

		this->preloadGlyphs("�~!@#$%^&*()_+`1234567890-=QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm|<>?,./:;\"'}{][ \\");
	}

	void Font::preloadGlyphs(std::string chars) {
		auto charsIter = chars.begin();
		while (charsIter < chars.end()) {
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
		auto fnt = std::find_if(this->_glyphs.begin(), this->_glyphs.end(), [codepoint](auto glyph) { return glyph.codepoint == codepoint; });
		if (fnt == this->_glyphs.end()) return this->_glyphs.front();
		return *fnt;
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

		auto beginIter = text.begin();
		auto endIter = utf8::find_invalid(text.begin(), text.end());

		while (beginIter != endIter) {
			uint32_t point = utf8::next(beginIter, endIter);
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

	bgfx::TextureHandle& Font::getHandle(const Glyph& g) {
		if (this->_engine == nullptr) throw std::runtime_error("[RawrBox-FONT] Text engine is null");
		return this->_engine->getAtlas(g.atlasID)->getHandle();
	}
	// -------

	// GLYPH LOADING -----
	Glyph Font::loadGlyph(FT_ULong character) {
		if (this->hasGlyph(character)) return getGlyph(character);

		FT_UInt charIndx = FT_Get_Char_Index(this->face, character);
		if (charIndx == 0) return {};

		if (FT_Load_Glyph(this->face, charIndx, FT_LOAD_NO_BITMAP | FT_LOAD_TARGET_(FT_RENDER_MODE_NORMAL)) != FT_Err_Ok) return {};
		std::vector<unsigned char> buffer = this->generateGlyph();

		auto atlas = this->_engine->requestAtlas(bitmapW, bitmapR);
		if (atlas.second == nullptr) throw std::runtime_error("[RawrBox-FONT] Failed to generate / get atlas texture");

		auto& atlasNode = atlas.second->addSprite(bitmapW, bitmapR, buffer);

		Glyph glyph{
		    atlas.first,
		    character,
		    charIndx,
		    {static_cast<float>(this->face->glyph->metrics.horiBearingX) / 64.f,
			static_cast<float>(this->face->glyph->metrics.horiBearingY) / 64.f},
		    {static_cast<float>(this->face->glyph->advance.x >> 6),
			static_cast<float>(this->face->glyph->advance.y >> 6)},
		    {atlasNode.x / static_cast<float>(atlas.second->size),
			atlasNode.y / static_cast<float>(atlas.second->size)},
		    {(atlasNode.x + atlasNode.width) / static_cast<float>(atlas.second->size),
			(atlasNode.y + atlasNode.height) / static_cast<float>(atlas.second->size)},
		    {static_cast<float>(this->face->glyph->metrics.width) / 64.f,
			static_cast<float>(this->face->glyph->metrics.height) / 64.f},
		};

		this->_glyphs.push_back(glyph);
		return glyph;
	}

	std::vector<unsigned char> Font::generateGlyph() {
		FT_Glyph glyphDescFill = nullptr;
		if (FT_Get_Glyph(this->face->glyph, &glyphDescFill) != FT_Err_Ok) return {};
		if (FT_Glyph_To_Bitmap(&glyphDescFill, FT_RENDER_MODE_NORMAL, nullptr, true) != FT_Err_Ok) return {};

		auto glyph_bitmap = reinterpret_cast<FT_BitmapGlyph>(glyphDescFill);
		FT_Bitmap* bitmap_fill = &glyph_bitmap->bitmap;
		if (bitmap_fill == nullptr) return {};

		bitmapW = bitmap_fill->width;
		bitmapR = bitmap_fill->rows;

		auto buffer = std::vector<unsigned char>(bitmapW * bitmapR * 2, 0); // * 2 -> 2 color channels (red and green)
		for (unsigned int i = 0; i < bitmapW * bitmapR; ++i)
			buffer[i * 2] = bitmap_fill->buffer[i]; // + 0 -> 1st color channel

		FT_Done_Glyph(glyphDescFill); // Release
		return buffer;
	}
	// --------
} // namespace rawrBox
