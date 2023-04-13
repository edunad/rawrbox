#include <rawrbox/render/text/engine.h>

#include <stdexcept>
#include <fmt/format.h>

namespace rawrBox {
	uint32_t TextEngine::atlasID = 0;

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
		this->_atlas.clear();

		TextEngine::atlasID = 0;

		if (FT_Done_FreeType(ft) != 0) throw std::runtime_error("[RawrBox-Freetype] Failed to clean up freetype");
		ft = nullptr;
	}

	std::pair<uint32_t, rawrBox::TextureAtlas*> TextEngine::requestAtlas(int width, int height) {
		// Try to find a spot
		for(auto& at: this->_atlas) {
			if(at.second->canInsertNode(width, height)) return {at.first, at.second.get()};
		}

		// Ok, make a new atlas then
		auto id = TextEngine::atlasID++;
		auto atlas = std::make_unique<rawrBox::TextureAtlas>(128, bgfx::TextureFormat::RG8);
		atlas->upload();

		this->_atlas.emplace(id, std::move(atlas));
		return {id, this->_atlas[id].get()};
	}

	rawrBox::TextureAtlas* TextEngine::getAtlas(uint32_t id) {
		auto fnd = this->_atlas.find(id);
		if(fnd == this->_atlas.end()) throw std::runtime_error(fmt::format("[RawrBox-Freetype] Failed to find atlas id '{}'", id));

		return fnd->second.get();
	}

	rawrBox::Font& TextEngine::load(std::string filename, uint32_t size) {
		return *this->_fonts.emplace_back(std::make_unique<rawrBox::Font>(this, filename, size));
	}
}
