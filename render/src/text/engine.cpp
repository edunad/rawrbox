#include <rawrbox/render/text/engine.h>

#include <stdexcept>
#include <fmt/format.h>

namespace rawrBox {
	TextEngine::~TextEngine() { destroy(); }
	TextEngine::TextEngine() { initialize(); }

	void TextEngine::initialize() {
		if (ft != nullptr) throw std::runtime_error("[RawrBox-Freetype] Freetype already initialized");
		if (FT_Init_FreeType(&ft) != 0) throw std::runtime_error("[RawrBox-Freetype] Failed to initialize Freetype");

		// FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
		// FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
		unsigned char lcd_weights[5] = {0x10, 0x40, 0x70, 0x40, 0x10};
		FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_DEFAULT);
		FT_Library_SetLcdFilterWeights(ft, lcd_weights);
	}

	void TextEngine::destroy() {
		if (ft == nullptr) return;
		this->_fonts.clear();

		if (FT_Done_FreeType(ft) != 0) throw std::runtime_error("[RawrBox-Freetype] Failed to clean up freetype");
		ft = nullptr;
	}

	rawrBox::Font& TextEngine::load(std::string filename, uint32_t size, uint32_t flags) {
		return *this->_fonts.emplace_back(std::make_unique<rawrBox::Font>(ft, filename, size, flags));
	}
}
