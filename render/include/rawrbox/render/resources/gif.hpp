#pragma once

#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceGIF : public rawrbox::Resource {

	public:
		std::shared_ptr<rawrbox::TextureGIF> texture = nullptr;
		~ResourceGIF() override;

		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;
	};

	class GIFLoader : public rawrbox::Loader {
	public:
		~GIFLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
