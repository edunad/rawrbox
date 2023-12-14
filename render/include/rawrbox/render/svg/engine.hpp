#pragma once
#include <rawrbox/render/textures/image.hpp>

#include <lunasvg.h>

#include <filesystem>
#include <unordered_map>

namespace rawrbox {
	class SVGEngine {
		static std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> _svgs;
		static std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> _renderedSVGS;

		static lunasvg::Document* internalLoad(const std::filesystem::path& filename);

	public:
		static void shutdown();
		static rawrbox::TextureBase* load(const std::filesystem::path& filename, const rawrbox::Vector2i& size);
	};
} // namespace rawrbox
