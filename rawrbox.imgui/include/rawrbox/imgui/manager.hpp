#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <DynamicBuffer.hpp>
#include <RefCntAutoPtr.hpp>

#include <Buffer.h>

struct ImDrawData;
namespace rawrbox {
	class IMGUIManager {
	private:
		// TEXTURES ----
		static std::unique_ptr<rawrbox::TextureImage> _imguiFontTexture;
		// -------

		static void renderDrawData(ImDrawData* data);

	public:
		static void init(bool darkTheme = true);

		static void load();

		static void clear();
		static void render();

		static void shutdown(rawrbox::ENGINE_THREADS thread);
	};
} // namespace rawrbox
