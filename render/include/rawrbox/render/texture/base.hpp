#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {

	enum class TEXTURE_UV {
		UV_NONE = 0,
		UV_FLIP_U,
		UV_FLIP_V,
		UV_FLIP_UV
	};

	class TextureBase {
	protected:
		bgfx::TextureHandle _handle = BGFX_INVALID_HANDLE;
		rawrbox::Vector2i _size;

		int _channels = 0;
		uint64_t _flags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;

		rawrbox::TEXTURE_UV _textureUV = rawrbox::TEXTURE_UV::UV_NONE;

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

		virtual void update();
		virtual std::array<float, 4> getData();
	};
} // namespace rawrbox
