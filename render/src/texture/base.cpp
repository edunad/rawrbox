#include <rawrbox/math/utils/color.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <fmt/format.h>

#include "rawrbox/render/utils/pipeline.hpp"

namespace rawrbox {
	TextureBase::~TextureBase() {
		if (this->_failedToLoad) return; // Don't delete the fallback
		RAWRBOX_DESTROY(this->_handle);
	}

	void TextureBase::loadFallback() {
		this->_failedToLoad = true;
		this->_handle = rawrbox::MISSING_TEXTURE->getHandle();
	}

	void TextureBase::updateSampler() {
		if (this->_handle == nullptr) return;
		this->_handle->SetSampler(this->getSampler());
	}

	// UTILS ---
	bool TextureBase::hasTransparency() const { return this->_channels == 4 && this->_transparent; }

	const rawrbox::Vector2i& TextureBase::getSize() const { return this->_size; }

	int TextureBase::getChannels() const { return this->_channels; }

	bool TextureBase::isValid() const { return this->_handle != nullptr; }

	Diligent::ITexture* TextureBase::getTexture() const { return this->_tex; }
	Diligent::ITextureView* TextureBase::getHandle() const { return this->_handle; }

	Diligent::ISampler* TextureBase::getSampler() {
		return this->_sampler == nullptr ? rawrbox::PipelineUtils::defaultSampler : this->_sampler;
	}
	void TextureBase::setSampler(Diligent::SamplerDesc desc) {
		uint32_t id = desc.AddressU << 6 | desc.AddressV << 3 | desc.AddressW;
		this->_sampler = rawrbox::PipelineUtils::registerSampler(id, desc);

		this->updateSampler();
	}

	void TextureBase::setTextureUV(rawrbox::TEXTURE_UV mode) { this->_textureUV = mode; }
	rawrbox::TEXTURE_UV TextureBase::getTextureUV() const { return this->_textureUV; }

	const std::string& TextureBase::getName() const { return this->_name; }
	void TextureBase::setName(const std::string& name) { this->_name = fmt::format("RawrBox::Texture::{}", name); }

	void TextureBase::setSRGB(bool set) { this->_sRGB = set; }
	// ----

	void TextureBase::update() {}
	std::array<float, 4> TextureBase::getData() const { return {static_cast<float>(this->_textureUV)}; }

	void TextureBase::upload(Diligent::TEXTURE_FORMAT format, bool dynamic) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it

		// Try to determine texture format
		if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) {
			switch (this->_channels) {
				case 1:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_A8_UNORM;
					break;
				case 2:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM;
					break;
				default:
				case 3:
				case 4:
					format = this->_sRGB ? Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM_SRGB : Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM;
					break;
			}
		}

		if (this->_channels == 0) {
			switch (format) {
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_A8_UNORM:
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM:
					this->_channels = 1;
					break;
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM:
					this->_channels = 2;
					break;
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM:
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM_SRGB:
				default:
					this->_channels = 4;
					break;
			}
		}

		if (this->_channels == 3) { // No RGB8
			this->_channels = 4;
			this->_pixels = rawrbox::ColorUtils::setChannels(3, 4, this->_size.x, this->_size.y, this->_pixels);
		}

		if (this->_pixels.empty()) {
			this->_pixels.resize(this->_size.x * this->_size.y * this->_channels);
			std::memset(this->_pixels.data(), 0, this->_pixels.size());
		}

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_size.x;
		desc.Height = this->_size.y;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.Name = this->_name.c_str();

		Diligent::TextureSubResData pSubResource;
		pSubResource.pData = this->_pixels.data();
		pSubResource.Stride = this->_size.x * this->_channels;

		Diligent::TextureData data;
		data.pSubResources = &pSubResource;
		data.NumSubresources = 1;

		rawrbox::RENDERER->device()->CreateTexture(desc, &data, &this->_tex);
		if (this->_tex == nullptr) throw std::runtime_error(fmt::format("[RawrBox-TextureBase] Failed to create texture '{}'", this->_name));

		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		this->updateSampler();
	}
} // namespace rawrbox
