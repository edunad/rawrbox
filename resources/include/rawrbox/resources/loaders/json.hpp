#pragma once

#include <rawrbox/resources/loader.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace rawrbox {
	class ResourceJSON : public rawrbox::Resource {
		std::unique_ptr<nlohmann::json> _json = nullptr;

	public:
		ResourceJSON() = default;
		ResourceJSON(const ResourceJSON&) = delete;
		ResourceJSON(ResourceJSON&&) = delete;
		ResourceJSON& operator=(const ResourceJSON&) = delete;
		ResourceJSON& operator=(ResourceJSON&&) = delete;
		~ResourceJSON() override;

		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] nlohmann::json* get() const;
	};

	class JSONLoader : public rawrbox::Loader {
	public:
		JSONLoader() = default;
		JSONLoader(const JSONLoader&) = delete;
		JSONLoader(JSONLoader&&) = delete;
		JSONLoader& operator=(const JSONLoader&) = delete;
		JSONLoader& operator=(JSONLoader&&) = delete;
		~JSONLoader() override = default;

		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
