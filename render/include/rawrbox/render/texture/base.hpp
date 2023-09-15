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

		bool _failedToLoad = false;
		virtual void loadFallback();

	public:
		TextureBase() = default;
		TextureBase(const TextureBase&) = default;
		TextureBase(TextureBase&&) = delete;
		TextureBase& operator=(TextureBase&&) = delete;
		TextureBase& operator=(const TextureBase&) = delete;
		virtual ~TextureBase();

		// UTILS----
		[[nodiscard]] virtual bool hasTransparency() const;
		[[nodiscard]] virtual const rawrbox::Vector2i& getSize() const;
		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual const bgfx::TextureHandle& getHandle() const;
		[[nodiscard]] virtual std::array<float, 4> getData() const;

		virtual void setTextureUV(rawrbox::TEXTURE_UV mode);
		[[nodiscard]] virtual rawrbox::TEXTURE_UV getTextureUV() const;
		// -----

		virtual void setFlags(uint64_t flags);
		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::Count) = 0;

		virtual void update();
	};
} // namespace rawrbox
