#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/render/text/font.hpp>

#include <bx/platform.h>
#include <fmt/format.h>
#include <utf8.h>

#if BX_PLATFORM_WINDOWS
	#include <windows.h>
#endif

#include <array>
#include <bit>
#include <filesystem>
#include <iostream>
#include <utility>

namespace rawrbox {

	std::string Font::getFontInSystem(const std::string& path) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
		return fmt::format("/usr/share/fonts/{}", path);
#elif BX_PLATFORM_WINDOWS
		std::array<TCHAR, MAX_PATH> windir = {};

		GetWindowsDirectory(windir.data(), MAX_PATH);
		return fmt::format("{}\\Fonts\\{}", windir.data(), path);
#endif
	}

	Font::~Font() {
		if (FT_Done_Face(this->face) != 0) fmt::print(stderr, "[RawrBox-Font] Failed to clean up font\n");
	}

	Font::Font(std::string _filename, uint32_t _size, FT_Render_Mode renderMode) : _file(std::move(_filename)), _mode(renderMode), size(_size) {

		// Check our own content
		if (!std::filesystem::exists(this->_file)) {
			this->_file = this->getFontInSystem(this->_file);

			// Not found on content & system? Load fallback
			if (!std::filesystem::exists(this->_file)) {
				fmt::print("  └── Loading fallback font!\n");
				this->_file = this->getFontInSystem("cour.ttf"); // Fallback
										 // TODO: CHECK FALLBACK ON LINUX
				if (!std::filesystem::exists(this->_file)) throw std::runtime_error(fmt::format("[RawrBox-Font] Failed to load font '{}'", this->_file));
			}
		}

		if (FT_New_Face(rawrbox::TextEngine::ft, this->_file.c_str(), 0, &this->face) != FT_Err_Ok) {
			throw std::runtime_error(fmt::format("[RawrBox-Font] Failed to load font '{}'", this->_file));
		}

		FT_Size_RequestRec req;
		auto pixel_size = (FT_Long)std::round(this->size * 64.0F);
		if (pixel_size < 1)
			pixel_size = 1;

		req.type = FT_SIZE_REQUEST_TYPE_NOMINAL;
		req.width = pixel_size;
		req.height = pixel_size;
		req.horiResolution = 0;
		req.vertResolution = 0;

		FT_Request_Size(this->face, &req);
		FT_Select_Charmap(this->face, FT_ENCODING_UNICODE);

		this->addChars("�~!@#$%^&*()_+`1234567890-=QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm|<>?,./:;\"'}{][ \\");
	}

	void Font::addChars(const std::string& chars) {
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
		if (fnt == this->_glyphs.end()) return this->_glyphs.front(); // Return the unknown one
		return *fnt;
	}

	float Font::getKerning(const Glyph& left, const Glyph& right) const {
		FT_Vector kerning;

		FT_Get_Kerning(this->face, left.glyphIndex, right.glyphIndex, FT_KERNING_UNFITTED, &kerning);
		return static_cast<float>(kerning.x >> 6);
	}

	rawrbox::Vector2 Font::getStringSize(const std::string& text) const {
		const float lineheight = this->getLineHeight();
		if (text.empty()) return {0, lineheight};

		rawrbox::Vector2 total;
		rawrbox::Vector2 pos;

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

			if (!this->hasGlyph(point)) continue;

			const auto& glyph = this->getGlyph(point);
			auto maxh = std::max(static_cast<float>(glyph.size.y), lineheight);

			if (prevGlyph != nullptr) pos.x += getKerning(glyph, *prevGlyph);
			pos.x += glyph.advance.x;

			if (pos.x > total.x) total.x = pos.x;
			if (pos.y + maxh > total.y) total.y = pos.y + maxh;

			prevGlyph = &glyph;
		}

		return total;
	}

	std::shared_ptr<rawrbox::TextureAtlas> Font::getAtlasTexture(const Glyph& g) {
		return rawrbox::TextEngine::getAtlas(g.atlasID);
	}

	// -------

	// GLYPH LOADING -----
	Glyph Font::loadGlyph(FT_ULong character) {
		if (this->hasGlyph(character)) return getGlyph(character);

		FT_UInt charIndx = FT_Get_Char_Index(this->face, character);
		if (charIndx == 0) return {};

		if (FT_Load_Glyph(this->face, charIndx, FT_LOAD_TARGET_(this->_mode) | FT_LOAD_FORCE_AUTOHINT) != FT_Err_Ok) return {};
		std::vector<unsigned char> buffer = this->generateGlyph();

		auto atlas = rawrbox::TextEngine::requestAtlas(bitmapW, bitmapR, bgfx::TextureFormat::RG8);
		if (atlas.second == nullptr) throw std::runtime_error("[RawrBox-FONT] Failed to generate / get atlas texture");

		auto& atlasNode = atlas.second->addSprite(bitmapW, bitmapR, buffer);

		Glyph glyph{
		    atlas.first,
		    character,
		    charIndx,
		    {static_cast<float>(this->face->glyph->metrics.horiBearingX) / 64.F, static_cast<float>(this->face->glyph->metrics.horiBearingY) / 64.F},
		    {static_cast<float>(this->face->glyph->advance.x >> 6), static_cast<float>(this->face->glyph->advance.y >> 6)},
		    {atlasNode.x / static_cast<float>(atlas.second->size), atlasNode.y / static_cast<float>(atlas.second->size)},
		    {(atlasNode.x + atlasNode.width) / static_cast<float>(atlas.second->size), (atlasNode.y + atlasNode.height) / static_cast<float>(atlas.second->size)},
		    {static_cast<int>(this->face->glyph->metrics.width) / 64, static_cast<int>(this->face->glyph->metrics.height) / 64},
		};

		this->_glyphs.push_back(glyph);
		return glyph;
	}

	std::vector<unsigned char> Font::generateGlyph() {
		FT_Glyph glyphDescFill = nullptr;
		if (FT_Get_Glyph(this->face->glyph, &glyphDescFill) != FT_Err_Ok) return {};
		if (FT_Glyph_To_Bitmap(&glyphDescFill, this->_mode, nullptr, true) != FT_Err_Ok) return {};

		auto glyph_bitmap = std::bit_cast<FT_BitmapGlyph>(glyphDescFill);
		FT_Bitmap* bitmap_fill = &glyph_bitmap->bitmap;
		if (bitmap_fill == nullptr) return {};

		bitmapW = bitmap_fill->width;
		bitmapR = bitmap_fill->rows;

		auto siz = bitmapW * bitmapR * 2;
		auto buffer = std::vector<unsigned char>(siz, 0); // * 2 -> 2 color channels (red and green)
		for (unsigned int i = 0; i < bitmapW * bitmapR; ++i)
			buffer[i * 2] = bitmap_fill->buffer[i]; // + 0 -> 1st color channel

		FT_Done_Glyph(glyphDescFill); // Release
		return buffer;
	}
	// --------

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
