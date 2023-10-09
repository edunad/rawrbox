#include <rawrbox/render_temp/svg/engine.hpp>

#include <fmt/format.h>

namespace rawrbox {
	// VARS ----
	std::unordered_map<std::string, std::unique_ptr<lunasvg::Document>> SVGEngine::_svgs = {};
	std::unordered_map<std::string, std::unique_ptr<rawrbox::TextureImage>> SVGEngine::_renderedSVGS = {};
	//--------

	lunasvg::Document* SVGEngine::internalLoad(const std::filesystem::path& filename) {
		auto name = filename.generic_string();

		auto fnd = _svgs.find(name);
		if (fnd != _svgs.end()) {
			return fnd->second.get();
		}

		auto svg = lunasvg::Document::loadFromFile(name);
		if (svg == nullptr) throw std::runtime_error(fmt::format("[RawrBox-SVGEngine] Failed to load '{}'", name));

		auto ptr = svg.get();
		_svgs[name] = std::move(svg);
		return ptr;
	}

	void SVGEngine::shutdown() {
		_svgs.clear();
		_renderedSVGS.clear();
	}

	rawrbox::TextureBase* SVGEngine::load(const std::filesystem::path& filename, const rawrbox::Vector2i& size) {
		auto id = fmt::format("{}-{}x{}", filename.generic_string(), size.x, size.y);
		auto fnd = _renderedSVGS.find(id);
		if (fnd != _renderedSVGS.end()) return fnd->second.get();

		auto doc = SVGEngine::internalLoad(filename);
		if (doc == nullptr) throw std::runtime_error(fmt::format("[RawrBox-SVGEngine] Failed to load '{}'", filename.generic_string()));

		auto img = doc->renderToBitmap(size.x, size.y);
		auto texture = std::make_unique<rawrbox::TextureImage>(size, img.data(), 4);
		texture->upload();

		auto ptr = texture.get();
		_renderedSVGS[id] = std::move(texture);

		return ptr;
	}

} // namespace rawrbox