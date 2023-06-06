#pragma once

#include <rawrbox/render/text/font.hpp>
#include <rawrbox/render/texture/atlas.hpp>

#include <filesystem>
#include <map>
#include <utility>

namespace rawrbox {

	class TextEngine {

		static std::map<std::string, std::unique_ptr<rawrbox::Font>> _fonts;
		static std::map<uint32_t, std::unique_ptr<rawrbox::TextureAtlas>> _atlas;

		static std::string getFontInSystem(const std::filesystem::path& path);

	public:
		static uint32_t atlasID;

		static void shutdown();

		static std::pair<uint32_t, rawrbox::TextureAtlas*> requestAtlas(int width, int height, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8);
		static rawrbox::TextureAtlas* getAtlas(uint32_t id);

		static rawrbox::Font* load(const std::filesystem::path& filename, uint32_t size, uint32_t index = 0);
	};
} // namespace rawrbox
