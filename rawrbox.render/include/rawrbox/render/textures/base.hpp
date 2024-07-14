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
	enum class TEXTURE_TYPE {
		PIXEL,
		VERTEX
	};

	class TextureBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _handle;
		Diligent::RefCntAutoPtr<Diligent::ITexture> _tex;
		rawrbox::Vector2u _size = {};

		int _channels = 0;

		uint32_t _textureID = 0; // Default to missing texture, it's always reserved to 0
		uint32_t _depthTextureID = 0;
		uint32_t _slice = 0;

		std::vector<uint8_t> _pixels = {};

		rawrbox::TEXTURE_TYPE _type = rawrbox::TEXTURE_TYPE::PIXEL;
		Diligent::ISampler* _sampler = nullptr;

		bool _failedToLoad = false;
		bool _transparent = false;
		bool _sRGB = false;
		bool _registered = false;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-Texture");
		// -------------

		std::string _name;
		virtual void loadFallback();
		virtual void updateSampler();

		virtual void tryGetFormatChannels(Diligent::TEXTURE_FORMAT& format, int& channels);

	public:
		TextureBase() = default;
		TextureBase(const TextureBase&) = delete;
		TextureBase(TextureBase&&) = delete;
		TextureBase& operator=(TextureBase&&) = delete;
		TextureBase& operator=(const TextureBase&) = delete;
		virtual ~TextureBase();

		// UTILS---
		[[nodiscard]] virtual const std::vector<uint8_t>& getPixels() const;

		[[nodiscard]] virtual bool hasTransparency() const;
		[[nodiscard]] virtual const rawrbox::Vector2u& getSize() const;

		[[nodiscard]] virtual int getChannels() const;

		[[nodiscard]] virtual bool isValid() const;
		[[nodiscard]] virtual bool isRegistered() const;

		[[nodiscard]] virtual uint32_t getDepthTextureID() const;
		[[nodiscard]] virtual uint32_t getTextureID() const;
		virtual void setDepthTextureID(uint32_t id);
		virtual void setTextureID(uint32_t id);

		[[nodiscard]] virtual Diligent::ITexture* getTexture() const;
		[[nodiscard]] virtual Diligent::ITextureView* getHandle() const;

		[[nodiscard]] virtual Diligent::ISampler* getSampler();
		virtual void setSampler(Diligent::SamplerDesc desc);

		virtual void setType(rawrbox::TEXTURE_TYPE type);
		[[nodiscard]] virtual rawrbox::TEXTURE_TYPE getType() const;

		[[nodiscard]] virtual const std::string& getName() const;
		virtual void setName(const std::string& name);

		virtual void setSRGB(bool set);

		virtual void setSlice(uint32_t id);
		[[nodiscard]] virtual uint32_t getSlice() const;
		// -----

		virtual void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false);

		virtual void update();
		[[nodiscard]] virtual bool requiresUpdate() const;
	};
} // namespace rawrbox
