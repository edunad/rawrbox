#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class TextureBase {
	protected:
		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;
		rawrbox::Vector2i _size;

		int _channels = 0;
		uint64_t _flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;

	public:
		TextureBase() = default;
		TextureBase(const TextureBase&) = default;

		TextureBase(TextureBase&&) = delete;
		TextureBase& operator=(TextureBase&&) = delete;
		TextureBase& operator=(const TextureBase&) = delete;

		virtual ~TextureBase();

		// UTILS----
		[[nodiscard]] virtual bool hasTransparency() const;
		[[nodiscard]] const virtual rawrbox::Vector2i& getSize() const;
		[[nodiscard]] virtual bool valid() const;
		[[nodiscard]] const virtual bgfx::TextureHandle& getHandle() const;
		// -----

		virtual void setFlags(uint64_t flags);
		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) = 0;
	};
} // namespace rawrbox
