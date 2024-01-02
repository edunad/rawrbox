
#include <rawrbox/assimp/resources/model.hpp>

namespace rawrbox {
	// Resource ----
	ResourceAssimp::~ResourceAssimp() { this->_model.reset(); }

	bool ResourceAssimp::load(const std::vector<uint8_t>& buffer) {
		this->_model = std::make_unique<rawrbox::AssimpImporter>(flags);
		this->_model->load(this->filePath, buffer);

		return true;
	}

	rawrbox::AssimpImporter* ResourceAssimp::get() const { return this->_model.get(); }
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> AssimpLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceAssimp>();
	}

	bool AssimpLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".obj" ||
		       fileExtention == ".fbx" ||
		       fileExtention == ".dae" ||
		       fileExtention == ".3ds" ||
		       fileExtention == ".glb" ||
		       // fileExtention == ".mdl" ||
		       fileExtention == ".gltf" ||
		       fileExtention == ".md5mesh";
	}

	bool AssimpLoader::supportsBuffer() { return true; }
	// -------

} // namespace rawrbox
