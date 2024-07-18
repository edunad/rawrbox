#include <rawrbox/math/utils/color.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TextureBase::~TextureBase() {
		if (this->_failedToLoad) return; // Don't delete the fallback
		if (this->_handle != nullptr) {
			rawrbox::BindlessManager::unregisterTexture(*this);
			this->_textureID = 0;
		}

		RAWRBOX_DESTROY(this->_handle);
	}

	void TextureBase::loadFallback() {
		this->_failedToLoad = true;

		auto missing = _type == TEXTURE_TYPE::VERTEX ? rawrbox::MISSING_VERTEX_TEXTURE : rawrbox::MISSING_TEXTURE;

		this->_data.frames = missing->getData().frames;
		this->_handle = missing->getHandle();
		this->_textureID = missing->getTextureID();
	}

	void TextureBase::updateSampler() {
		if (this->_handle == nullptr) return;
		this->_handle->SetSampler(this->getSampler());
	}

	void TextureBase::tryGetFormatChannels(Diligent::TEXTURE_FORMAT& format, uint8_t& channels) {
		if (format == Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN) {
			switch (channels) {
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

		if (channels == 0U) {
			switch (format) {
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_A8_UNORM:
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_R8_UNORM:
					channels = 1U;
					break;
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RG8_UNORM:
					channels = 2U;
					break;
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM:
				case Diligent::TEXTURE_FORMAT::TEX_FORMAT_RGBA8_UNORM_SRGB:
				default:
					channels = 4U;
					break;
			}
		}

		// No RGB8, replace with SRGB
		if (this->_data.channels == 3U) {
			this->_data.channels = 4U;

			auto& pixels = this->_data.pixels();
			if (!pixels.empty()) {
				pixels = rawrbox::ColorUtils::setChannels(3U, 4U, this->_data.size.x, this->_data.size.y, pixels);
			}
		}
	}

	// UTILS ---
	void TextureBase::setID(uint64_t id) { this->_id = id; }
	uint64_t TextureBase::getID() const { return this->_id; }

	const rawrbox::ImageData& TextureBase::getData() const { return this->_data; }
	const std::vector<uint8_t>& TextureBase::getPixels(size_t index) const {
		if (index > this->_data.frames.size()) throw this->_logger->error("Pixel data from frame index {} not found", index);
		return this->_data.frames[index].pixels;
	}

	bool TextureBase::hasTransparency() const { return this->_data.channels == 4U && this->_transparent; }
	const rawrbox::Vector2u& TextureBase::getSize() const { return this->_data.size; }

	uint8_t TextureBase::getChannels() const { return this->_data.channels; }

	bool TextureBase::isValid() const { return this->getHandle() != nullptr; }
	bool TextureBase::isRegistered() const { return this->_registered; }

	uint32_t TextureBase::getDepthTextureID() const { return this->_depthTextureID; }
	uint32_t TextureBase::getTextureID() const { return this->_textureID; }
	void TextureBase::setDepthTextureID(uint32_t id) { this->_depthTextureID = id; }
	void TextureBase::setTextureID(uint32_t id) {
		this->_textureID = id;
		this->_registered = true;
	}

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

	void TextureBase::setType(rawrbox::TEXTURE_TYPE type) { this->_type = type; }
	rawrbox::TEXTURE_TYPE TextureBase::getType() const { return this->_type; }

	const std::string& TextureBase::getName() const { return this->_name; }
	void TextureBase::setName(const std::string& name) { this->_name = fmt::format("RawrBox::Texture::{}", name); }

	void TextureBase::setSRGB(bool set) { this->_sRGB = set; }

	void TextureBase::setSlice(uint32_t id) { this->_slice = id; }
	uint32_t TextureBase::getSlice() const { return this->_slice; }
	// ----

	void TextureBase::upload(Diligent::TEXTURE_FORMAT format, bool dynamic) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it
		if (!this->_data.valid()) throw this->_logger->error("Cannot upload invalid image data");
		if (this->_data.total() > 2048U) throw this->_logger->error("Cannot upload more than 2048 image frames");

		// Try to determine texture format
		this->tryGetFormatChannels(format, this->_data.channels);
		// --------------------------------

		if (this->_data.empty()) {
			this->_data.createFrame();
		} else {
			this->_transparent = this->_data.transparent();
		}

		Diligent::TextureDesc desc;
		desc.Type = Diligent::RESOURCE_DIM_TEX_2D_ARRAY;
		desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
		desc.Usage = dynamic ? Diligent::USAGE_DEFAULT : Diligent::USAGE_IMMUTABLE;
		desc.CPUAccessFlags = Diligent::CPU_ACCESS_NONE;
		desc.Width = this->_data.size.x;
		desc.Height = this->_data.size.y;
		desc.MipLevels = 1;
		desc.Format = format;
		desc.Name = this->_name.c_str();

		std::vector<Diligent::TextureSubResData> subresData = {};
		// NOLINTBEGIN(cppcoreguidelines-pro-type-union-access)
		desc.ArraySize = static_cast<uint32_t>(this->_data.total());
		// NOLINTEND(cppcoreguidelines-pro-type-union-access)

		subresData.resize(desc.ArraySize);
		for (uint32_t slice = 0; slice < subresData.size(); slice++) {
			auto& res = subresData[slice];

			res.pData = this->_data.frames[slice].pixels.data();
			res.Stride = desc.Width * this->_data.channels;
		}

		Diligent::TextureData data;
		data.pSubResources = subresData.data();
		data.NumSubresources = static_cast<uint32_t>(subresData.size());

		rawrbox::RENDERER->device()->CreateTexture(desc, &data, &this->_tex);
		if (this->_tex == nullptr) throw this->_logger->error("Failed to create texture '{}'", this->_name);

		// Get handles --
		this->_handle = this->_tex->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE);
		// -------

		rawrbox::runOnRenderThread([this]() {
			rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
			rawrbox::BindlessManager::registerTexture(*this);
		});
	}

	void TextureBase::update() {}
	bool TextureBase::requiresUpdate() const { return false; }

} // namespace rawrbox
