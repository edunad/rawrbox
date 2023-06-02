#pragma once

#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceGIF : public rawrbox::Resource {

	public:
		std::shared_ptr<rawrbox::TextureGIF> texture = nullptr;
		~ResourceGIF() override { this->texture.reset(); }

		bool load(const std::vector<uint8_t>& buffer) override {
			this->texture = std::make_shared<rawrbox::TextureGIF>(this->filePath, buffer);
			return true;
		}

		void upload() override {
			if (this->texture == nullptr) return;
			this->texture->upload();
		}
	};

	class GIFLoader : public rawrbox::Loader {
	public:
		~GIFLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override {
			return std::make_unique<rawrbox::ResourceGIF>();
		}

		bool canLoad(const std::string& fileExtention) override {
			return fileExtention == ".gif";
		}
	};
} // namespace rawrbox
