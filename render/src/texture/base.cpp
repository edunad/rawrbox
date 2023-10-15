#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureBase::~TextureBase() {
		if (this->_failedToLoad) return; // Don't delete the fallback
		RAWRBOX_DESTROY(this->_handle);
	}

	void TextureBase::loadFallback() {
		this->_failedToLoad = true;
		this->_handle = rawrbox::MISSING_TEXTURE->getHandle();
	}

	// UTILS ---
	bool TextureBase::hasTransparency() const { return this->_channels == 4 && this->_transparent; }
	const rawrbox::Vector2i& TextureBase::getSize() const { return this->_size; }
	int TextureBase::getChannels() const { return this->_channels; }
	bool TextureBase::isValid() const { return this->_handle != nullptr; }
	Diligent::ITextureView* TextureBase::getHandle() const { return this->_handle; }
	void TextureBase::setTextureUV(rawrbox::TEXTURE_UV mode) { this->_textureUV = mode; }
	rawrbox::TEXTURE_UV TextureBase::getTextureUV() const { return this->_textureUV; }

	void TextureBase::setName(const std::string& name) { this->_name = name; }
	// ----

	void TextureBase::update() {}
	std::array<float, 4> TextureBase::getData() const { return {static_cast<float>(this->_textureUV)}; }

	void TextureBase::upload(Diligent::TEXTURE_FORMAT format) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it

		// Try to determine texture format
		if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) {
			switch (this->_channels) {
				case 1:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM;
				case 2:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM;
				default:
				case 3:
				case 4:
					format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM;
			}
		}

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_size.x;
		desc.Height = this->_size.y;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.Name = fmt::format("RawrBox::Texture::{}", this->_name).c_str();

		Diligent::TextureSubResData pSubResource;
		pSubResource.pData = this->_pixels.data();
		pSubResource.Stride = this->_size.x * this->_channels;

		Diligent::TextureData data;
		data.pSubResources = &pSubResource;
		data.NumSubresources = 1;

		rawrbox::RENDERER->device->CreateTexture(desc, &data, &this->_tex);
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
	}
} // namespace rawrbox
