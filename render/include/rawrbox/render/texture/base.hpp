#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/Texture.h>
#include <Graphics/GraphicsEngine/interface/TextureView.h>

#include <vector>

namespace rawrbox {

	enum class TEXTURE_UV {
		UV_NONE = 0,
		UV_FLIP_U,
		UV_FLIP_V,
		UV_FLIP_UV
	};

	class TextureBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _handle;
		Diligent::RefCntAutoPtr<Diligent::ITexture> _tex;
		rawrbox::Vector2i _size = {};

		int _channels = 0;
		std::vector<uint8_t> _pixels = {};
		rawrbox::TEXTURE_UV _textureUV = rawrbox::TEXTURE_UV::UV_NONE;

		bool _failedToLoad = false;
		bool _transparent = false;
		bool _sRGB = true;

		std::string _name = "";
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
		[[nodiscard]] virtual int getChannels() const;
		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual Diligent::ITextureView* getHandle() const;
		[[nodiscard]] virtual std::array<float, 4> getData() const;

		virtual void setTextureUV(rawrbox::TEXTURE_UV mode);
		[[nodiscard]] virtual rawrbox::TEXTURE_UV getTextureUV() const;

		[[nodiscard]] virtual const std::string& getName() const;
		virtual void setName(const std::string& name);

		virtual void setSRGB(bool set);
		// -----

		virtual void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false);
		virtual void update();
	};
} // namespace rawrbox
