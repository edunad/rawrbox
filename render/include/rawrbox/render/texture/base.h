#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/color.hpp>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureBase {
	protected:
		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;
		rawrBox::Vector2i _size;

		int _channels = 4;

	public:
		TextureBase() = default;
		TextureBase(const TextureBase& t) = default;
		virtual ~TextureBase();

		virtual void upload() = 0;
		virtual bgfx::TextureHandle& getHandle();
	};
}
