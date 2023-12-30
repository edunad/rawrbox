
#include <rawrbox/render/resources/font.hpp>

namespace rawrbox {
	// Resource ----
	rawrbox::Font* ResourceFont::getSize(uint16_t size, uint32_t /*flags*/) {
		return rawrbox::TextEngine::load(filePath.generic_string(), size);
	}
	// -------

	// Loader ----
	std::unique_ptr<rawrbox::Resource> FontLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceFont>();
	}

	bool FontLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".ttf" ||
		       fileExtention == ".otf" ||
		       fileExtention == ".ttc" ||
		       fileExtention == ".pfa" ||
		       fileExtention == ".pfb" ||
		       fileExtention == ".ttc";
	}
	// -------
} // namespace rawrbox
