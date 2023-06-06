#pragma once

#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceFont : public rawrbox::Resource {

	public:
		rawrbox::Font* getSize(int size, uint32_t flags = 0);
	};

	class FontLoader : public rawrbox::Loader {
	public:
		FontLoader() = default;
		~FontLoader() override;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
