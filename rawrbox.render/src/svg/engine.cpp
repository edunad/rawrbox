#include <rawrbox/render/svg/engine.hpp>
#include <rawrbox/utils/file.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// VARS ----
	std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> SVGEngine::_svgs = {};
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> SVGEngine::_renderedSVGS = {};

	// LOGGER ------
	std::unique_ptr<rawrbox::Logger> SVGEngine::_logger = std::make_unique<rawrbox::Logger>("RawrBox-SVGEngine");
	// -------------
	//--------

	lunasvg::Document* SVGEngine::internalLoad(const std::filesystem::path& filename) {
		auto name = filename.generic_string();

		auto fnd = _svgs.find(name);
		if (fnd != _svgs.end()) {
			return fnd->second.get();
		}

		auto svg = parse(rawrbox::FileUtils::getRawData(filename));
		if (svg == nullptr) RAWRBOX_CRITICAL("Failed to load '{}'", name);

		auto* ptr = svg.get();
		_svgs[name] = std::move(svg);
		return ptr;
	}

	std::unique_ptr<lunasvg::Document> SVGEngine::parse(const std::vector<uint8_t>& buffer) { // Remove trailing null
		size_t offset = 0;
		if (buffer.size() >= 3 && buffer[0] == 0xEF && buffer[1] == 0xBB && buffer[2] == 0xBF) offset = 3;

		size_t length = buffer.size();
		while (length > offset && buffer[length - 1] == '\0')
			length--;

		return lunasvg::Document::loadFromData(reinterpret_cast<const char*>(buffer.data() + offset), length - offset);
	}

	void SVGEngine::shutdown() {
		_svgs.clear();
		_renderedSVGS.clear();
	}

	bool SVGEngine::preLoad(const std::filesystem::path& filename, const std::vector<uint8_t>& buffer) {
		auto name = filename.generic_string();

		auto fnd = _svgs.find(name);
		if (fnd != _svgs.end()) return true; // Already loaded

		auto svg = parse(buffer);
		if (svg == nullptr) return false;

		_svgs[name] = std::move(svg);
		return true;
	}

	rawrbox::TextureBase* SVGEngine::load(const std::filesystem::path& filename, const rawrbox::Vector2u& size) {
		auto id = fmt::format("{}-{}x{}", filename.generic_string(), size.x, size.y);
		auto fnd = _renderedSVGS.find(id);
		if (fnd != _renderedSVGS.end()) return fnd->second.get();

		auto* doc = SVGEngine::internalLoad(filename);
		if (doc == nullptr) RAWRBOX_CRITICAL("Failed to load '{}'", filename.generic_string());

		auto img = doc->renderToBitmap(size.x, size.y);
		auto texture = std::make_unique<rawrbox::TextureImage>(size, img.data());
		texture->setName("SVG");
		texture->upload();

		auto* ptr = texture.get();
		_renderedSVGS[id] = std::move(texture);

		return ptr;
	}

} // namespace rawrbox
