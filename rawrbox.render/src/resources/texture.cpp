
#include <rawrbox/render/resources/texture.hpp>
#include <rawrbox/render/textures/atlas.hpp>
#include <rawrbox/render/textures/gif.hpp>
#include <rawrbox/render/textures/image.hpp>
#include <rawrbox/render/textures/ktx2.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>
#include <rawrbox/render/textures/webp.hpp>

namespace rawrbox {
	// Resource ----
	ResourceTexture::~ResourceTexture() { this->_texture.reset(); }

	bool ResourceTexture::load(const std::vector<uint8_t>& buffer) {
		const rawrbox::TEXTURE_TYPE type = this->filePath.generic_string().rfind(".vertex.") != std::string::npos ? rawrbox::TEXTURE_TYPE::VERTEX : rawrbox::TEXTURE_TYPE::PIXEL;
		const auto imageType = rawrbox::TextureUtils::getImageType(buffer);
		const uint32_t spriteSize = this->flags & ~(rawrbox::TextureFlags::DYNAMIC | rawrbox::TextureFlags::NO_MIPMAPS);

		if (imageType == rawrbox::ImageType::IMAGE_GIF) {
			this->_texture = std::make_unique<rawrbox::TextureGIF>(this->filePath, buffer);
		} else if (imageType == rawrbox::ImageType::IMAGE_WEBP) {
			this->_texture = std::make_unique<rawrbox::TextureWEBP>(this->filePath, buffer);
		} else if (imageType == rawrbox::ImageType::IMAGE_KTX2) {
			if (spriteSize != 0U) RAWRBOX_CRITICAL("Atlas '{}' cannot be KTX2", this->filePath.generic_string());
			this->_texture = std::make_unique<rawrbox::TextureKTX2>(this->filePath, buffer);
		} else if (spriteSize != 0U) {
			this->_texture = std::make_unique<rawrbox::TextureAtlas>(this->filePath, buffer, spriteSize);
		} else {
			this->_texture = std::make_unique<rawrbox::TextureImage>(this->filePath, buffer);
		}

		this->_texture->setType(type);
		this->_texture->setName(this->filePath.filename().generic_string());
		return true;
	}

	void ResourceTexture::upload() {
		if (this->_texture == nullptr) return;
		this->_texture->upload(Diligent::TEX_FORMAT_UNKNOWN, this->flags);
	}

	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> TextureLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceTexture>();
	}

	bool TextureLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".png" ||
		       fileExtention == ".bmp" ||
		       fileExtention == ".jpg" ||
		       fileExtention == ".gif" ||
		       fileExtention == ".tga" ||
		       fileExtention == ".webp" ||
		       fileExtention == ".ktx2" ||
		       fileExtention == ".jpeg";
	}

	bool TextureLoader::supportsBuffer(const std::string& /*fileExtention*/) { return true; }
	// -------
} // namespace rawrbox