#pragma once

#include <rawrbox/render/texture/gif.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceGIF : public rawrbox::Resource {

		std::unique_ptr<rawrbox::TextureGIF> _texture = nullptr;

	public:
		~ResourceGIF() override;

		bool load(const std::vector<uint8_t>& buffer) override;
		void upload() override;

		[[nodiscard]] rawrbox::TextureGIF* get() const;
	};

	class GIFLoader : public rawrbox::Loader {
	public:
		~GIFLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
