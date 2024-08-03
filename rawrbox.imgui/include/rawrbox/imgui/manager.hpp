#pragma once

#include <rawrbox/render/window.hpp>

#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

struct ImDrawData;
namespace rawrbox {
	class IMGUIManager {
	private:
		// TEXTURES ----
		static std::unique_ptr<rawrbox::TextureImage> _imguiFontTexture;
		// -------

		static rawrbox::Window* _window;
		static rawrbox::RendererBase* _renderer;

		static void renderDrawData(ImDrawData* data);

	public:
		static void init(rawrbox::Window& window, bool darkTheme = true);

		static void load();

		static void clear();
		static void render();

		static void shutdown();
	};
} // namespace rawrbox
