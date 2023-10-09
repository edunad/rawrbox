#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render_temp/texture/pack.hpp>

#include <bgfx/bgfx.h>

#include <memory>
#include <string>
#include <unordered_map>

struct stbtt_fontinfo;

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
		uint16_t packID = 0;
		uint32_t codePoint = 0;

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
		std::shared_ptr<stbtt_fontinfo> _font = nullptr; // unique_ptr does not like incomplete types
		std::unordered_map<uint32_t, std::unique_ptr<rawrbox::Glyph>> _glyphs = {};

		float _scale = 0.F;
		float _pixelSize = 0.F;

		int16_t _widthPadding = 0;
		int16_t _heightPadding = 0;
		rawrbox::FontInfo _info = {};

		// INTERNAL ---
		virtual void loadFontInfo();

		std::unique_ptr<rawrbox::Glyph> bakeGlyphAlpha(uint32_t codePoint);
		virtual void generateGlyph(uint32_t codePoint);
		// ----

	public:
		virtual ~Font();

		Font(int16_t widthPadding = 6, int16_t heightPadding = 6);
		Font(Font&&) = delete;
		Font& operator=(Font&&) = delete;
		Font(const Font&) = delete;
		Font& operator=(const Font&) = delete;

		// LOADING ---
		virtual void load(const std::vector<uint8_t>& buffer, uint32_t pixelHeight, int32_t fontIndex = 0);
		virtual void addChars(const std::string& chars);
		// ----

		// UTILS ---
		[[nodiscard]] virtual const rawrbox::FontInfo getFontInfo() const;

		[[nodiscard]] virtual bool hasGlyph(uint32_t codepoint) const;
		[[nodiscard]] virtual rawrbox::Glyph* getGlyph(uint32_t codepoint) const;

		[[nodiscard]] virtual float getSize() const;
		[[nodiscard]] virtual float getScale() const;
		[[nodiscard]] virtual float getLineHeight() const;
		[[nodiscard]] virtual float getKerning(uint32_t prevCodePoint, uint32_t nextCodePoint) const;

		[[nodiscard]] virtual rawrbox::Vector2f getStringSize(const std::string& text) const;
		[[nodiscard]] virtual rawrbox::TexturePack* getPackTexture(rawrbox::Glyph* g) const;

		virtual void render(const std::string& text, const rawrbox::Vector2f& pos, bool yIsUp, std::function<void(rawrbox::Glyph*, float, float, float, float)> render) const;
		// ----

		// GLOBAL UTILS ---
		static size_t getByteCount(const std::string& text, size_t characterPosition);
		static size_t getCharacterCount(const std::string& text);
		static std::string toUTF8(const std::wstring text);
		// -------------*/
	};

} // namespace rawrbox
