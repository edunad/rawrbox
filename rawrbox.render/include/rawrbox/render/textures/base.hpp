#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <RefCntAutoPtr.hpp>

#include <Sampler.h>
#include <Texture.h>
#include <TextureView.h>

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

		uint32_t _textureID = 0;
		std::vector<uint8_t> _pixels = {};

		rawrbox::TEXTURE_UV _textureUV = rawrbox::TEXTURE_UV::UV_NONE;
		Diligent::ISampler* _sampler = nullptr;

		bool _failedToLoad = false;
		bool _transparent = false;
		bool _sRGB = false;

		std::string _name = "";
		virtual void loadFallback();
		virtual void updateSampler();

		virtual void tryGetFormatChannels(Diligent::TEXTURE_FORMAT& format, int& channels);

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
		[[nodiscard]] virtual uint32_t getTextureID() const;

		[[nodiscard]] virtual Diligent::ITexture* getTexture() const;
		[[nodiscard]] virtual Diligent::ITextureView* getHandle() const;
		[[nodiscard]] virtual std::array<float, 4> getData() const;

		[[nodiscard]] virtual Diligent::ISampler* getSampler();
		virtual void setSampler(Diligent::SamplerDesc desc);

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
