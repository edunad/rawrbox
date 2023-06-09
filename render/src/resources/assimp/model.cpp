
#include <rawrbox/render/resources/assimp/model.hpp>

namespace rawrbox {
	// Resource ----
	bool ResourceAssimp::load(const std::vector<uint8_t>& buffer) {
		this->_model = std::make_unique<rawrbox::AssimpImporter>(flags);
		this->_model->load(this->filePath, buffer);

		return true;
	}

	rawrbox::AssimpImporter* ResourceAssimp::get() const { return this->_model.get(); }
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
