#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/textures/pack.hpp>

#include <filesystem>
#include <map>
#include <utility>

namespace rawrbox {

	class TextEngine {

		static std::map<std::string, std::unique_ptr<rawrbox::Font>> _fonts;
		static std::map<uint16_t, std::unique_ptr<rawrbox::TexturePack>> _packs;

		static std::string getFontInSystem(const std::filesystem::path& path);

	public:
		static uint16_t packID;

		static void shutdown();

		static std::pair<uint16_t, rawrbox::TexturePack*> requestPack(uint16_t width, uint16_t height, Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM);
		static rawrbox::TexturePack* getPack(uint16_t id);

		static rawrbox::Font* load(const std::filesystem::path& filename, uint16_t size, uint16_t index = 0);
	};
} // namespace rawrbox
