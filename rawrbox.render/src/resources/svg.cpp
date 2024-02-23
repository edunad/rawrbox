
#include <rawrbox/render/resources/svg.hpp>
#include <rawrbox/render/svg/engine.hpp>

namespace rawrbox {
	// Resource ----

	bool ResourceSVG::load(const std::vector<uint8_t>& buffer) {
		return rawrbox::SVGEngine::preLoad(filePath.generic_string(), buffer);
	}

	rawrbox::TextureBase* ResourceSVG::get(const rawrbox::Vector2i& size, uint32_t /*flags*/) {
		return rawrbox::SVGEngine::load(filePath.generic_string(), size);
	}
	// -------

	// Loader ----
	SVGLoader::~SVGLoader() { rawrbox::SVGEngine::shutdown(); };
	std::unique_ptr<rawrbox::Resource> SVGLoader::createEntry() {
		return std::make_unique<rawrbox::ResourceSVG>();
	}

	bool SVGLoader::canLoad(const std::string& fileExtention) {
		return fileExtention == ".svg";
	}

	bool SVGLoader::supportsBuffer(const std::string& /*fileExtention*/) { return true; }
	// -------
} // namespace rawrbox
