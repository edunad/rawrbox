#pragma once

#include <rawrbox/render/model/assimp/assimp_importer.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceAssimp : public rawrbox::Resource {
	public:
		std::shared_ptr<rawrbox::AssimpImporter> model = nullptr;

		~ResourceAssimp() override {
			this->model = nullptr;
		}

		bool load(const std::vector<uint8_t>& buffer) override {
			this->model = std::make_shared<rawrbox::AssimpImporter>(flags);
			this->model->load(this->filePath, buffer);

			return true;
		}
	};

	class AssimpLoader : public rawrbox::Loader {
	public:
		~AssimpLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override {
			return std::make_unique<rawrbox::ResourceAssimp>();
		}

		bool canLoad(const std::string& fileExtention) override {
			return fileExtention == ".obj" ||
			       fileExtention == ".fbx" ||
			       fileExtention == ".dae" ||
			       fileExtention == ".3ds" ||
			       fileExtention == ".mdl" ||
			       fileExtention == ".gltf" ||
			       fileExtention == ".md5mesh";
		}
	};
} // namespace rawrbox
