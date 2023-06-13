#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/text/font.hpp>

#include <fmt/format.h>
#include <utf8.h>

#include <array>
#include <bit>
#include <filesystem>
#include <iostream>
#include <string>

namespace rawrbox {
	Font::~Font() {
		if (this->_font == nullptr) return;
		this->_font.reset();
		this->_glyphs.clear();
	}

	Font::Font(const std::vector<uint8_t>& buffer, uint32_t pixelHeight, int32_t fontIndex, int16_t widthPadding, int16_t heightPadding) : _widthPadding(widthPadding), _heightPadding(heightPadding), _info({}) {
		int offset = stbtt_GetFontOffsetForIndex(buffer.data(), fontIndex); // Get the offset for `otf` fonts

		// Load
		this->_font = std::make_unique<stbtt_fontinfo>();
		if (!stbtt_InitFont(this->_font.get(), buffer.data(), offset)) throw std::runtime_error("[RawrBox-Font] Failed to load font");
		this->_scale = stbtt_ScaleForMappingEmToPixels(this->_font.get(), static_cast<float>(pixelHeight));
		this->_pixelSize = static_cast<float>(pixelHeight);

		this->loadFontInfo();
		this->addChars("�~!@#$%^&*()_+`1234567890-=QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm|<>?,./:;\"'}{][ \\░▒▓█");
	}

	// INTERNAL ---
	void Font::loadFontInfo() {
		if (this->_font == nullptr) throw std::runtime_error("[RawrBox-Font] Font not loaded");

		int ascent = 0;
		int descent = 0;
		int lineGap = 0;
		stbtt_GetFontVMetrics(this->_font.get(), &ascent, &descent, &lineGap);

		int x0 = 0, y0 = 0, x1 = 0, y1 = 0;
		stbtt_GetFontBoundingBox(this->_font.get(), &x0, &y0, &x1, &y1);

		this->_info = {};
		this->_info.scale = 1.0F;
		this->_info.ascender = std::round(ascent * this->_scale);
		this->_info.descender = std::round(descent * this->_scale);
		this->_info.lineGap = std::round(lineGap * this->_scale);
		this->_info.maxAdvanceWidth = std::round((y1 - y0) * this->_scale);

		this->_info.underlinePosition = (x1 - x0) * this->_scale - ascent;
		this->_info.underlineThickness = (x1 - x0) * this->_scale / 24.F;
	}

	std::unique_ptr<rawrbox::Glyph> Font::bakeGlyphAlpha(uint16_t codePoint) {
		if (this->_font == nullptr) throw std::runtime_error("[RawrBox-Font] Font not loaded");

		int32_t ascent = 0, descent = 0, lineGap = 0;
		stbtt_GetFontVMetrics(this->_font.get(), &ascent, &descent, &lineGap);

		int32_t advance = 0, lsb = 0;
		stbtt_GetCodepointHMetrics(this->_font.get(), codePoint, &advance, &lsb);

		const float scale = this->_scale;
		int32_t x0 = 0, y0 = 0, x1 = 0, y1 = 0;
		stbtt_GetCodepointBitmapBox(this->_font.get(), codePoint, scale, scale, &x0, &y0, &x1, &y1);

		const int32_t ww = x1 - x0;
		const int32_t hh = y1 - y0;

		std::unique_ptr<rawrbox::Glyph> glyph = std::make_unique<rawrbox::Glyph>();
		glyph->codePoint = codePoint;
		glyph->offset = {static_cast<float>(x0), static_cast<float>(y0)};
		glyph->size = {static_cast<float>(ww), static_cast<float>(hh)};
		glyph->advance = {std::round(static_cast<float>(advance) * scale), std::round((static_cast<float>(ascent + descent + lineGap)) * scale)};

		// Bitmap ----
		uint32_t bpp = 1;
		uint32_t dstPitch = ww * bpp;

		std::vector<uint8_t> buffer = {};
		buffer.resize(ww * hh * sizeof(uint8_t));
		stbtt_MakeCodepointBitmap(this->_font.get(), buffer.data(), ww, hh, dstPitch, scale, scale, codePoint);
		// ----

		auto atlas = rawrbox::TextEngine::requestAtlas(ww, hh, bgfx::TextureFormat::A8); // FONT_TYPE_ALPHA
		if (atlas.second == nullptr) throw std::runtime_error("[RawrBox-FONT] Failed to generate / get atlas texture");

		auto& atlasNode = atlas.second->addSprite(ww, hh, buffer);

		glyph->atlasID = atlas.first;
		glyph->textureTopLeft = {atlasNode.x / static_cast<float>(atlas.second->size), atlasNode.y / static_cast<float>(atlas.second->size)};
		glyph->textureBottomRight = {(atlasNode.x + atlasNode.width) / static_cast<float>(atlas.second->size), (atlasNode.y + atlasNode.height) / static_cast<float>(atlas.second->size)};

		glyph->scale = this->_info.scale;

		glyph->advance *= this->_info.scale;
		glyph->offset *= this->_info.scale;
		glyph->size *= this->_info.scale;

		return glyph;
	}

	void Font::generateGlyph(uint16_t codePoint) {
		if (this->hasGlyph(codePoint)) return;
		this->_glyphs[codePoint] = this->bakeGlyphAlpha(codePoint);
	}
	// ----

	// LOADING ---
	void Font::addChars(const std::string& chars) {
		auto charsIter = chars.begin();
		while (charsIter < chars.end()) {
			this->generateGlyph(utf8::next(charsIter, chars.end()));
		}
	}
	// ----

	// UTILS ---
	const rawrbox::FontInfo Font::getFontInfo() const { return this->_info; }

	bool Font::hasGlyph(uint16_t codepoint) const {
		return this->_glyphs.find(codepoint) != this->_glyphs.end();
	}

	rawrbox::Glyph* Font::getGlyph(uint16_t codepoint) const {
		auto fnd = this->_glyphs.find(codepoint);
		if (fnd == this->_glyphs.end()) return this->_glyphs.find(65533)->second.get(); // �
		return fnd->second.get();
	}

	float Font::getSize() const { return this->_pixelSize; }
	float Font::getScale() const { return this->_scale; }
	float Font::getLineHeight() const { return this->_info.ascender - this->_info.descender + this->_info.lineGap; }
	float Font::getKerning(uint16_t prevCodePoint, uint16_t nextCodePoint) const {
		if (this->_font == nullptr || (!this->_font->kern && !this->_font->gpos)) return 0; // no kerning
		return this->_info.scale * static_cast<float>(stbtt__GetGlyphKernInfoAdvance(this->_font.get(), prevCodePoint, nextCodePoint));
	}

	rawrbox::Vector2f Font::getStringSize(const std::string& text) const {
		rawrbox::Vector2f size = {};
		if (this->_font == nullptr) return size;

		const float lineHeight = this->getLineHeight();
		size.y = lineHeight;

		if (text.empty()) return size;

		uint16_t prevCodePoint = 0;
		float cursorX = 0.F;

		auto beginIter = text.begin();
		auto endIter = utf8::find_invalid(text.begin(), text.end()); // Find invalid utf8

		while (beginIter != endIter) {
			uint16_t point = utf8::next(beginIter, endIter); // get codepoint

			const auto glyph = this->getGlyph(point);
			if (glyph == nullptr) continue;

			if (point == '\n') {
				size.y += lineHeight;
				cursorX = 0;
				prevCodePoint = 0;
			}

			float kerning = this->getKerning(prevCodePoint, point);
			cursorX += kerning;
			cursorX += glyph->advance.x;

			if (cursorX > size.x) size.x = cursorX;
			prevCodePoint = point;
		}

		return size;
	}

	rawrbox::TextureAtlas* Font::getAtlasTexture(rawrbox::Glyph* g) const {
		if (g == nullptr) return nullptr;
		return rawrbox::TextEngine::getAtlas(g->atlasID);
	}

	void Font::render(const std::string& text, const rawrbox::Vector2f& pos, bool yIsUp, std::function<void(rawrbox::Glyph*, float, float, float, float)> renderGlyph) const {
		if (renderGlyph == nullptr) throw std::runtime_error("[RawrBox-FONT] Failed to render glyph! Missing 'renderGlyph' param");

		auto info = this->getFontInfo();
		const float lineHeight = this->getLineHeight();

		rawrbox::Vector2f cursor = {pos.x, pos.y + lineHeight + info.descender};
		uint16_t prevCodePoint = 0;

		auto beginIter = text.begin();
		auto endIter = utf8::find_invalid(text.begin(), text.end()); // Find invalid utf8
		while (beginIter != endIter) {
			uint16_t point = utf8::next(beginIter, endIter); // get codepoint
			if (point == L'\n') {
				if (!yIsUp) {
					cursor.y += lineHeight;
				} else {
					cursor.y -= lineHeight;
				}

				cursor.x = pos.x;
				prevCodePoint = 0;
				continue;
			}

			const auto glyph = this->getGlyph(point);
			float kerning = this->getKerning(prevCodePoint, point);
			cursor.x += kerning;

			float x0 = cursor.x + glyph->offset.x;
			float y0 = cursor.y + glyph->offset.y;
			float x1 = x0 + glyph->size.x;
			float y1 = y0 + glyph->size.y;

			renderGlyph(glyph, x0, y0, x1, y1);

			cursor.x += glyph->advance.x;
			prevCodePoint = point;
		}
	}
	// ----

	// GLOBAL UTILS ---
	size_t Font::getByteCount(const std::string& text, size_t characterPosition) {
		if (characterPosition <= 0) return 0;

		size_t count = 0;
		auto beginIter = text.begin();
		auto endIter = text.end();
		while (beginIter != endIter) {
			utf8::next(beginIter, endIter);

			count++;
			if (characterPosition == count) {
				return std::distance(text.begin(), beginIter);
			}
		}

		return text.size();
	}

	size_t Font::getCharacterCount(const std::string& text) {
		size_t count = 0;

		auto beginIter = text.begin();
		auto endIter = text.end();
		while (beginIter != endIter) {
			utf8::next(beginIter, endIter);
			count++;
		}

		return count;
	}

	std::string Font::toUTF8(const std::wstring text) {
		std::string result;

		utf8::utf16to8(text.begin(), text.end(), std::back_inserter(result));
		return result;
	}
	// -------------
} // namespace rawrbox
