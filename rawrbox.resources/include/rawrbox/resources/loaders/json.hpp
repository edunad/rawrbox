#pragma once

#include <rawrbox/resources/loader.hpp>

#include <glaze/glaze.hpp>

namespace rawrbox {
	class ResourceJSON : public rawrbox::Resource {
	protected:
		// PRIVATE ---
		static std::unique_ptr<rawrbox::Logger> _logger;
		// ------

		glz::generic _json = {};

	public:
		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] const glz::generic& get() const;
	};

	class JSONLoader : public rawrbox::Loader {
	public:
		JSONLoader() = default;
		JSONLoader(const JSONLoader&) = delete;
		JSONLoader(JSONLoader&&) = delete;
		JSONLoader& operator=(const JSONLoader&) = delete;
		JSONLoader& operator=(JSONLoader&&) = delete;
		~JSONLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry() override;

		bool canLoad(const std::string& fileExtention) override;
		bool supportsBuffer(const std::string& fileExtention) override;
	};
} // namespace rawrbox
