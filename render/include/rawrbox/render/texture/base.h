#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

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

		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) = 0;
		virtual bgfx::TextureHandle& getHandle();
		virtual bool valid();
	};
} // namespace rawrBox
