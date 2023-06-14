#pragma once

#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceTexture : public rawrbox::Resource {
		std::unique_ptr<rawrbox::TextureImage> _texture = nullptr;

	public:
		ResourceTexture() = default;
		ResourceTexture(const ResourceTexture&) = delete;
		ResourceTexture(ResourceTexture&&) = delete;
		ResourceTexture& operator=(const ResourceTexture&) = delete;
		ResourceTexture& operator=(ResourceTexture&&) = delete;
		~ResourceTexture() override;

		[[nodiscard]] rawrbox::TextureImage* get() const;
		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;
	};

	class TextureLoader : public rawrbox::Loader {
	public:
		TextureLoader() = default;
		TextureLoader(const TextureLoader&) = delete;
		TextureLoader(TextureLoader&&) = delete;
		TextureLoader& operator=(const TextureLoader&) = delete;
		TextureLoader& operator=(TextureLoader&&) = delete;
		~TextureLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
