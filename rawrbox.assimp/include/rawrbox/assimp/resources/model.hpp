#pragma once

#include <rawrbox/assimp/importer.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceAssimp : public rawrbox::Resource {
		std::unique_ptr<rawrbox::AssimpImporter> _model = nullptr;

	public:
		ResourceAssimp() = default;
		ResourceAssimp(const ResourceAssimp&) = delete;
		ResourceAssimp(ResourceAssimp&&) = delete;
		ResourceAssimp& operator=(const ResourceAssimp&) = delete;
		ResourceAssimp& operator=(ResourceAssimp&&) = delete;
		~ResourceAssimp() override;

		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] rawrbox::AssimpImporter* get() const;
	};

	class AssimpLoader : public rawrbox::Loader {
	public:
		AssimpLoader() = default;
		AssimpLoader(const AssimpLoader&) = delete;
		AssimpLoader(AssimpLoader&&) = delete;
		AssimpLoader& operator=(const AssimpLoader&) = delete;
		AssimpLoader& operator=(AssimpLoader&&) = delete;
		~AssimpLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry() override;

		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer(const std::string& fileExtention) override;
	};
} // namespace rawrbox
