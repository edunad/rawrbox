
#include <rawrbox/render_temp/resources/svg.hpp>
#include <rawrbox/render_temp/svg/engine.hpp>

namespace rawrbox {
	// Resource ----
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
	// -------
} // namespace rawrbox
