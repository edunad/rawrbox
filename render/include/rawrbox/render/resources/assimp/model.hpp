#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceAssimp : public rawrbox::Resource {
	public:
		std::shared_ptr<rawrbox::AssimpImporter> model = nullptr;

		~ResourceAssimp() override;
		bool load(const std::vector<uint8_t>& buffer) override;
	};

	class AssimpLoader : public rawrbox::Loader {
	public:
		~AssimpLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
