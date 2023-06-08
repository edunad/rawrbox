#pragma once

#include <rawrbox/render/texture/image.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceTexture : public rawrbox::Resource {
		std::unique_ptr<rawrbox::TextureImage> _texture = nullptr;

	public:
		[[nodiscard]] rawrbox::TextureImage* get() const;
		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;
	};

	class TextureLoader : public rawrbox::Loader {
	public:
		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
