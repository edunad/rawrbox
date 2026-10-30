#pragma once

#include <rawrbox/engine/engine.hpp>
#include <rawrbox/render/window.hpp>

#include <unordered_map>

struct ImDrawData;
namespace rawrbox {
	class IMGUIManager {
	private:
		// TEXTURES ----
		static std::unordered_map<int, std::unique_ptr<rawrbox::TextureImage>> _textures;
		// -------

		static void renderDrawData(ImDrawData* data);

	public:
		static void init(bool darkTheme = true);

		static void clear();
		static void render();

		static void shutdown(rawrbox::ENGINE_THREADS thread);
	};
} // namespace rawrbox