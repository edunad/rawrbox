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

	struct ImageFrame {
		float delay = 0.F; // User for animation
		std::vector<uint8_t> pixels = {};
	};

	struct ImageData {
		rawrbox::Vector2u size = {};
		uint8_t channels = 0U;

		std::vector<rawrbox::ImageFrame> frames = {};

		void createFrame() {
			rawrbox::ImageFrame frame = {};
			frame.pixels.resize(this->size.x * this->size.y * this->channels);
			std::memset(frame.pixels.data(), 0, frame.pixels.size()); // Fill it with empty pixels

			this->frames.emplace_back(frame);
		};

		void createFrame(const std::vector<uint8_t>& frame) { this->frames.push_back({0.F, frame}); };
		[[nodiscard]] std::vector<uint8_t>& pixels() { return this->frames.begin()->pixels; }
		[[nodiscard]] const std::vector<uint8_t>& pixels() const { return this->frames.begin()->pixels; }

		[[nodiscard]] bool valid() const { return channels != 0U && size > 0; }
		[[nodiscard]] bool empty() const { return this->frames.empty(); }

		[[nodiscard]] size_t total() const { return this->frames.size(); }
		[[nodiscard]] bool transparent() const {
			if (channels != 4U || this->frames.empty()) return false;

			for (const auto& frame : this->frames) {
				for (size_t o = 0; o < frame.pixels.size(); o += channels) {
					if (frame.pixels[o + 3] == 1.F) continue;
					return true;
				}
			}

			return false;
		}
	};

	class TextureBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _handle;
		Diligent::RefCntAutoPtr<Diligent::ITexture> _tex;

		rawrbox::ImageData _data = {};

		uint32_t _textureID = 0; // Default to missing texture, it's always reserved to 0
		uint32_t _depthTextureID = 0;
		uint32_t _slice = 0;

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

		virtual void tryGetFormatChannels(Diligent::TEXTURE_FORMAT& format, uint8_t& channels);

	public:
		TextureBase() = default;
		TextureBase(const TextureBase&) = delete;
		TextureBase(TextureBase&&) = delete;
		TextureBase& operator=(TextureBase&&) = delete;
		TextureBase& operator=(const TextureBase&) = delete;
		virtual ~TextureBase();

		// UTILS---
		[[nodiscard]] virtual const rawrbox::ImageData& getData() const;
		[[nodiscard]] virtual const std::vector<uint8_t>& getPixels(size_t index = 0) const;

		[[nodiscard]] virtual bool hasTransparency() const;
		[[nodiscard]] virtual const rawrbox::Vector2u& getSize() const;

		[[nodiscard]] virtual uint8_t getChannels() const;

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
