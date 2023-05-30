#include <rawrbox/render/text/engine.hpp>

#include <fmt/format.h>

#include <array>
#include <stdexcept>

namespace rawrbox {
	// VARS ----
	std::map<std::string, std::shared_ptr<rawrbox::Font>> TextEngine::_fonts = {};
	std::map<uint32_t, std::shared_ptr<rawrbox::TextureAtlas>> TextEngine::_atlas = {};

	// PUBLIC ----
	uint32_t TextEngine::atlasID = 0;
	FT_Library TextEngine::ft = nullptr;
	// ----------

	void TextEngine::initialize() {
		if (ft != nullptr) throw std::runtime_error("[RawrBox-Freetype] Freetype already initialized");
		if (FT_Init_FreeType(&ft) != 0) throw std::runtime_error("[RawrBox-Freetype] Failed to initialize Freetype");

		// FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
		// FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
		std::array<unsigned char, 5> lcd_weights = {0x10, 0x40, 0x70, 0x40, 0x10};

		FT_Library_SetLcdFilter(ft, FT_LCD_FILTER_DEFAULT);
		FT_Library_SetLcdFilterWeights(ft, lcd_weights.data());
	}

	void TextEngine::shutdown() {
		if (ft == nullptr) return;
		_fonts.clear();
		_atlas.clear();

		TextEngine::atlasID = 0;

		if (FT_Done_FreeType(ft) != 0) throw std::runtime_error("[RawrBox-Freetype] Failed to clean up freetype");
		ft = nullptr;
	}

	std::pair<uint32_t, std::shared_ptr<rawrbox::TextureAtlas>> TextEngine::requestAtlas(int width, int height, bgfx::TextureFormat::Enum format) {
		// Try to find a spot
		for (auto& at : _atlas) {
			if (at.second->canInsertNode(width, height)) return {at.first, at.second};
		}

		// Ok, make a new atlas then
		auto id = TextEngine::atlasID++;
		auto atlas = std::make_shared<rawrbox::TextureAtlas>(512);
		atlas->upload(format);

		_atlas.emplace(id, std::move(atlas));
		return {id, _atlas[id]};
	}

	std::shared_ptr<rawrbox::TextureAtlas> TextEngine::getAtlas(uint32_t id) {
		auto fnd = _atlas.find(id);
		if (fnd == _atlas.end()) throw std::runtime_error(fmt::format("[RawrBox-Freetype] Failed to find atlas id '{}'", id));
		return fnd->second;
	}

	std::weak_ptr<rawrbox::Font> TextEngine::load(std::string filename, uint32_t size) {
		std::string key = fmt::format("{}-{}", filename, size);

		auto fnd = _fonts.find(key);
		if (fnd != _fonts.end()) return fnd->second;

		_fonts[key] = std::make_shared<rawrbox::Font>(filename, size);
		return _fonts[key];
	}
} // namespace rawrbox
