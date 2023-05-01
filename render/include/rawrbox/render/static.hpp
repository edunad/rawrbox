#pragma once
#include <rawrbox/render/texture/flat.hpp>
#include <rawrbox/render/texture/missing.hpp>

#include <bgfx/bgfx.h>

#include <memory>

// NOLINTBEGIN(*)
#define RAWRBOX_DESTROY(HANDLE) \
	if (bgfx::isValid(HANDLE)) { \
		bgfx::destroy(HANDLE); \
		(HANDLE) = BGFX_INVALID_HANDLE; \
	}
// NOLINTEND(*)
namespace rawrBox {
	constexpr auto MAX_BONES_PER_VERTEX = 4;
	constexpr auto MAX_BONES_PER_MODEL = 200;
	constexpr auto MAX_LIGHTS = 12;

	extern bgfx::ViewId CURRENT_VIEW_ID;
	extern std::shared_ptr<rawrBox::TextureMissing> MISSING_TEXTURE;
	extern std::shared_ptr<rawrBox::TextureFlat> MISSING_SPECULAR_TEXTURE;
	extern std::shared_ptr<rawrBox::TextureFlat> WHITE_TEXTURE;
} // namespace rawrBox
