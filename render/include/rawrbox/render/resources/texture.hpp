#pragma once

#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceTexture : public rawrbox::Resource {
	public:
		std::shared_ptr<rawrbox::TextureImage> texture = nullptr;
		~ResourceTexture() override {
			this->texture = nullptr;
		}

		bool load(const std::vector<uint8_t>& buffer) override {
			this->texture = std::make_shared<rawrbox::TextureImage>(this->filePath, buffer);
			return true;
		}

		void upload() override {
			if (this->texture == nullptr) return;
			this->texture->upload();
		}
	};

	class TextureLoader : public rawrbox::Loader {
	public:
		~TextureLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override {
			return std::make_unique<rawrbox::ResourceTexture>();
		}

		bool canLoad(const std::string& fileExtention) override {
			return fileExtention == ".png" ||
			       fileExtention == ".bmp" ||
			       fileExtention == ".jpg" ||
			       fileExtention == ".jpeg";
		}
	};
} // namespace rawrbox
