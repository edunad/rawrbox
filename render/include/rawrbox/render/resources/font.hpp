#pragma once

#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/resources/loader.hpp>

#include <map>

namespace rawrbox {
	class ResourceFont : public rawrbox::Resource {

	public:
		std::shared_ptr<rawrbox::Font> getSize(int size, uint32_t flags = 0) {
			return rawrbox::TextEngine::load(filePath.generic_string(), size);
		}
	};

	class FontLoader : public rawrbox::Loader {
	public:
		FontLoader() = default;
		~FontLoader() override { rawrbox::TextEngine::shutdown(); };

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override {
			return std::make_unique<rawrbox::ResourceFont>();
		}

		bool canLoad(const std::string& fileExtention) override {
			return fileExtention == ".ttf" ||
			       fileExtention == ".otf" ||
			       fileExtention == ".ttc" ||
			       fileExtention == ".pfa" ||
			       fileExtention == ".pfb" ||
			       fileExtention == ".ttc";
		}
	};
} // namespace rawrbox
