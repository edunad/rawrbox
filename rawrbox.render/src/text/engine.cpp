#include <rawrbox/render/static.hpp>
#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/utils/path.hpp>

#include <fmt/format.h>

#include <array>

#if WIN32
	#include <windows.h>
#endif

namespace rawrbox {
	// PROTECTED ----
	std::map<std::string, std::unique_ptr<rawrbox::Font>> TextEngine::_fonts = {};
	std::map<uint16_t, std::unique_ptr<rawrbox::TexturePack>> TextEngine::_packs = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> TextEngine::_logger = std::make_unique<rawrbox::Logger>("RawrBox-TextEngine");
	// -------------
	// ------------

	// PUBLIC ----
	uint16_t TextEngine::packID = 0;
	// ----------

	std::string TextEngine::getFontInSystem(const std::filesystem::path& path) {
#if WIN32
		std::array<TCHAR, MAX_PATH> windir = {};

		GetWindowsDirectory(windir.data(), MAX_PATH);
		return fmt::format("{}\\Fonts\\{}", windir.data(), path.generic_string());
#else
		return fmt::format("/usr/share/fonts/{}", path.generic_string());
#endif
	}

	void TextEngine::shutdown() {
		_fonts.clear();
		_packs.clear();

		TextEngine::packID = 0;
	}

	std::pair<uint16_t, rawrbox::TexturePack*> TextEngine::requestPack(uint16_t width, uint16_t height, Diligent::TEXTURE_FORMAT format) {
		// Try to find a spot
		for (auto& at : _packs) {
			if (at.second->canInsertNode(width, height)) return {at.first, at.second.get()};
		}

		// Ok, make a new pack then
		auto id = TextEngine::packID++;
		auto pack = std::make_unique<rawrbox::TexturePack>(512U);
		pack->upload(format);

		_packs.emplace(id, std::move(pack));
		return {id, _packs[id].get()};
	}

	rawrbox::TexturePack* TextEngine::getPack(uint16_t id) {
		auto fnd = _packs.find(id);
		if (fnd == _packs.end()) throw _logger->error("Failed to find pack id '{}'", id);
		return fnd->second.get();
	}

	rawrbox::Font* TextEngine::load(const std::filesystem::path& filename, uint16_t size, uint32_t index) {
		std::string key = fmt::format("{}-{}", filename.generic_string(), size);

		// Check cache
		auto fnd = _fonts.find(key);
		if (fnd != _fonts.end()) return fnd->second.get();
		// ------

		// Check our own content
		auto pth = filename;
		if (!std::filesystem::exists(pth)) {
			pth = getFontInSystem(pth);

			// Not found on content & system? Load fallback
			if (!std::filesystem::exists(pth)) {
				_logger->warn("Failed to load '{}' ──> File not found\n  └── Loading fallback font!", filename.generic_string());
				return rawrbox::DEBUG_FONT_REGULAR;
			}
		}

		auto bytes = rawrbox::PathUtils::getRawData(pth);
		if (bytes.empty()) throw _logger->error("Failed to load font '{}'", filename.generic_string());

		_fonts[key] = std::make_unique<rawrbox::Font>(filename);
		_fonts[key]->load(bytes, size, index);

		return _fonts[key].get();
	}
} // namespace rawrbox
