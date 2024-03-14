#pragma once

#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/resources/loader.hpp>

namespace rawrbox {
	class ResourceSVG : public rawrbox::Resource {

	public:
		bool load(const std::vector<uint8_t>& buffer) override;
		rawrbox::TextureBase* get(const rawrbox::Vector2u& size, uint32_t flags = 0);
	};

	class SVGLoader : public rawrbox::Loader {
	public:
		SVGLoader() = default;
		SVGLoader(const SVGLoader&) = delete;
		SVGLoader(SVGLoader&&) = delete;
		SVGLoader& operator=(const SVGLoader&) = delete;
		SVGLoader& operator=(SVGLoader&&) = delete;

		~SVGLoader() override;

		std::unique_ptr<rawrbox::Resource> createEntry() override;

		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer(const std::string& fileExtention) override;
	};
} // namespace rawrbox
