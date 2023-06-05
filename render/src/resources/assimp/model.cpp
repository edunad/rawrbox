
#include <rawrbox/render/resources/assimp/model.hpp>

namespace rawrbox {
	// Resource ----
	ResourceAssimp::~ResourceAssimp() { this->model.reset(); }

	bool ResourceAssimp::load(const std::vector<uint8_t>& buffer) {
		this->model = std::make_shared<rawrbox::AssimpImporter>(flags);
		this->model->load(this->filePath, buffer);

		return true;
	}
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> AssimpLoader::createEntry(uint32_t flags) {
		return std::make_unique<rawrbox::ResourceAssimp>();
	}

	bool AssimpLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".obj" ||
		       fileExtention == ".fbx" ||
		       fileExtention == ".dae" ||
		       fileExtention == ".3ds" ||
		       fileExtention == ".mdl" ||
		       fileExtention == ".gltf" ||
		       fileExtention == ".md5mesh";
	}
	// -------

} // namespace rawrbox
