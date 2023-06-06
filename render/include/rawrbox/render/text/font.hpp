#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb/stb_truetype.hpp>

#include <bgfx/bgfx.h>

#include <memory>
#include <string>
#include <unordered_map>

namespace rawrbox {
	enum class Alignment {
		Left,
		Center,
		Right
	};

	struct FontInfo {
		/// The font height in pixel.
		uint16_t pixelSize;
		/// Rendering type used for the font.
		int16_t fontType;

		/// The pixel extents above the baseline in pixels (typically positive).
		float ascender;
		/// The extents below the baseline in pixels (typically negative).
		float descender;
		/// The spacing in pixels between one row's descent and the next row's ascent.
		float lineGap;
		/// This field gives the maximum horizontal cursor advance for all glyphs in the font.
		float maxAdvanceWidth;
		/// The thickness of the under/hover/strike-trough line in pixels.
		float underlineThickness;
		/// The position of the underline relatively to the baseline.
		float underlinePosition;

		/// Scale to apply to glyph data.
		float scale;
	};

	// Glyph metrics:
	// --------------
	//
	//                       xmin                     xmax
	//                        |                         |
	//                        |<-------- width -------->|
	//                        |                         |
	//              |         +-------------------------+----------------- ymax
	//              |         |    ggggggggg   ggggg    |     ^        ^
	//              |         |   g:::::::::ggg::::g    |     |        |
	//              |         |  g:::::::::::::::::g    |     |        |
	//              |         | g::::::ggggg::::::gg    |     |        |
	//              |         | g:::::g     g:::::g     |     |        |
	//    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
	//              |         | g:::::g     g:::::g     |     |        |
	//              |         | g::::::g    g:::::g     |     |        |
	//              |         | g:::::::ggggg:::::g     |     |        |
	//              |         |  g::::::::::::::::g     |     |      height
	//              |         |   gg::::::::::::::g     |     |        |
	//  baseline ---*---------|---- gggggggg::::::g-----*--------      |
	//            / |         |             g:::::g     |              |
	//     origin   |         | gggggg      g:::::g     |              |
	//              |         | g:::::gg   gg:::::g     |              |
	//              |         |  g::::::ggg:::::::g     |              |
	//              |         |   gg:::::::::::::g      |              |
	//              |         |     ggg::::::ggg        |              |
	//              |         |         gggggg          |              v
	//              |         +-------------------------+----------------- ymin
	//              |                                   |
	//              |------------- advance_x ---------->|

	struct Glyph {
		uint32_t atlasID = 0;
		uint16_t codePoint = 0;

		float scale = 0.F;

		rawrbox::Vector2f size = {};
		rawrbox::Vector2f offset = {};
		rawrbox::Vector2f advance = {};

		rawrbox::Vector2f textureTopLeft = {};
		rawrbox::Vector2f textureBottomRight = {};
	};

	class TextEngine;
	class Font {
	private:
		std::unique_ptr<stbtt_fontinfo> _font = nullptr;
		std::unordered_map<uint16_t, std::unique_ptr<rawrbox::Glyph>> _glyphs = {};

		float _scale = 0.F;
		float _pixelSize = 0.F;

		int16_t _widthPadding = 0;
		int16_t _heightPadding = 0;
		rawrbox::FontInfo _info = {};

		// INTERNAL ---
		void loadFontInfo();

		std::unique_ptr<rawrbox::Glyph> bakeGlyphAlpha(uint16_t codePoint);
		void generateGlyph(uint16_t codePoint);
		// ----

	public:
		~Font();

		Font(const std::vector<uint8_t>& buffer, uint32_t pixelHeight, int32_t fontIndex = 0, int16_t widthPadding = 6, int16_t heightPadding = 6);
		Font(Font&&) = delete;
		Font& operator=(Font&&) = delete;
		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;

		// LOADING ---
		void addChars(const std::string& chars);
		// ----

		// UTILS ---
		[[nodiscard]] const rawrbox::FontInfo getFontInfo() const;

		[[nodiscard]] bool hasGlyph(uint16_t codepoint) const;
		[[nodiscard]] rawrbox::Glyph* getGlyph(uint16_t codepoint) const;

		[[nodiscard]] float getSize() const;
		[[nodiscard]] float getScale() const;
		[[nodiscard]] float getLineHeight() const;
		[[nodiscard]] float getKerning(uint16_t prevCodePoint, uint16_t nextCodePoint) const;

		[[nodiscard]] rawrbox::Vector2f getStringSize(const std::string& text) const;
		[[nodiscard]] rawrbox::TextureAtlas* getAtlasTexture(rawrbox::Glyph* g) const;

		void render(const std::string& text, const rawrbox::Vector2f& pos, std::function<void(rawrbox::Glyph*, float, float, float, float)> render);
		// ----

		// GLOBAL UTILS ---
		static size_t getByteCount(const std::string& text, size_t characterPosition);
		static size_t getCharacterCount(const std::string& text);
		static std::string toUTF8(const std::wstring text);
		// -------------*/
	};

} // namespace rawrbox
