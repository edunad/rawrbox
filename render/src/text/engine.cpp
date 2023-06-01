#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/utils/path.hpp>

#include <bx/platform.h>
#include <fmt/format.h>

#include <array>
#include <stdexcept>

#if BX_PLATFORM_WINDOWS
	#include <windows.h>
#endif

namespace rawrbox {
	// VARS ----
	std::map<std::string, std::shared_ptr<rawrbox::Font>> TextEngine::_fonts = {};
	std::map<uint32_t, std::shared_ptr<rawrbox::TextureAtlas>> TextEngine::_atlas = {};

	// PUBLIC ----
	uint32_t TextEngine::atlasID = 0;
	// ----------

	std::string TextEngine::getFontInSystem(const std::filesystem::path& path) {
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
		return fmt::format("/usr/share/fonts/{}", path.generic_string());
#elif BX_PLATFORM_WINDOWS
		std::array<TCHAR, MAX_PATH> windir = {};

		GetWindowsDirectory(windir.data(), MAX_PATH);
		return fmt::format("{}\\Fonts\\{}", windir.data(), path.generic_string());
#endif
	}

	void TextEngine::shutdown() {
		_fonts.clear();
		_atlas.clear();

		TextEngine::atlasID = 0;
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

	std::shared_ptr<rawrbox::Font> TextEngine::load(const std::filesystem::path& filename, uint32_t size, uint32_t index) {
		std::string key = fmt::format("{}-{}", filename.generic_string(), size);
		// Check cache
		auto fnd = _fonts.find(key);
		if (fnd != _fonts.end()) return fnd->second;
		// ------

		// Check our own content
		auto pth = filename;
		if (!std::filesystem::exists(pth)) {
			pth = getFontInSystem(pth);

			// Not found on content & system? Load fallback
			if (!std::filesystem::exists(pth)) {
				fmt::print("  └── Loading fallback font!\n");
				pth = getFontInSystem("cour.ttf"); // Fallback
								   // TODO: CHECK FALLBACK ON LINUX
				if (!std::filesystem::exists(pth)) throw std::runtime_error(fmt::format("[RawrBox-Font] Failed to load font '{}'", filename.generic_string()));
			}
		}

		auto bytes = rawrbox::PathUtils::getRawData(pth);
		if (bytes.empty()) throw std::runtime_error(fmt::format("[RawrBox-Font] Failed to load font '{}'", filename.generic_string()));

		_fonts[key] = std::make_shared<rawrbox::Font>(bytes, size, index);
		return _fonts[key];
	}
} // namespace rawrbox
