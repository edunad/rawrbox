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
		bool load(const std::vector<uint8_t>& buffer) override;
		[[nodiscard]] nlohmann::json* get() const;
	};

	class JSONLoader : public rawrbox::Loader {
	public:
		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
