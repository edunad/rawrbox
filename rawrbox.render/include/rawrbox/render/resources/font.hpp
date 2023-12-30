#pragma once

#include <rawrbox/render/text/engine.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceFont : public rawrbox::Resource {

	public:
		rawrbox::Font* getSize(uint16_t size, uint32_t flags = 0);
	};

	class FontLoader : public rawrbox::Loader {
	public:
		std::unique_ptr<rawrbox::Resource> createEntry() override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
