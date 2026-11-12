
#include <rawrbox/gltf/resources/model.hpp>

namespace rawrbox {
	// Resource ----
	ResourceGLTF::~ResourceGLTF() { this->_model.reset(); }

	bool ResourceGLTF::load(const std::vector<uint8_t>& buffer) {
		auto model = std::make_unique<rawrbox::GLTFImporter>(this->flags);
		model->load(this->filePath, buffer);

		this->_model = std::move(model);
		return true;
	}

	rawrbox::GLTFImporter* ResourceGLTF::get() const { return this->_model.get(); }
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> GLTFLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceGLTF>();
	}

	bool GLTFLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".glb" ||
		       fileExtention == ".gltf";
	}

	bool GLTFLoader::supportsBuffer(const std::string& /*fileExtention*/) {
		return true;
	}
	// -------

} // namespace rawrbox
