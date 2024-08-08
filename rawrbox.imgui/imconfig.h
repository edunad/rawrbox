#pragma once

namespace rawrbox {
	class TextureBase;
}

#define ImTextureID rawrbox::TextureBase*
#define ImDrawIdx   unsigned int
#define IMGUI_OVERRIDE_DRAWVERT_STRUCT_LAYOUT \
	struct ImDrawVert { \
		ImU32 textureID; \
		ImVec2 pos; \
		ImU32 col; \
		ImVec2 uv; \
		ImVec2 __padding__; \
	};
