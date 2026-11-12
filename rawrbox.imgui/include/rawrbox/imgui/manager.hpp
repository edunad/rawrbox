#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/textures/image.hpp>
#include <rawrbox/render/window.hpp>

#include <imgui.h>

#ifdef RAWRBOX_IMGUI_RICHMD
	#include <imgui_rich_md/rich_md.h>
#endif

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>

struct ImDrawData;
namespace rawrbox {
	struct IMGUISettings {
		bool dark = true;
		bool docking = true;

		std::optional<std::filesystem::path> ini = "imgui.ini";
		ImGuiConfigFlags flags = ImGuiConfigFlags_None;

#ifdef RAWRBOX_IMGUI_RICHMD
		std::function<std::optional<RichMd::MarkdownImage>(const std::string&)> onImage = nullptr;
		std::function<void(const std::string&)> onLink = nullptr;
#endif
	};

	class IMGUIManager {
	private:
		// TEXTURES ----
		static std::unordered_map<int, std::unique_ptr<rawrbox::TextureImage>> _textures;
		// -------

		static std::string _init;

		static void renderDrawData(ImDrawData* data);

	public:
		static void init(const rawrbox::IMGUISettings& settings = {});

		static void clear();
		static void render();

		static void shutdown(rawrbox::ENGINE_THREADS thread);
	};
} // namespace rawrbox
