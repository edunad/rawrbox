#pragma once

#include <rawrbox/resources/loader.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

namespace rawrbox {
	class ResourceJSON : public rawrbox::Resource {

	public:
		std::shared_ptr<nlohmann::json> json = nullptr;
		~ResourceJSON() override;

		bool load(const std::vector<uint8_t>& buffer) override;
	};

	class JSONLoader : public rawrbox::Loader {
	public:
		std::unique_ptr<rawrbox::Resource> createEntry(uint32_t flags = 0) override;
		bool canLoad(const std::string& fileExtention) override;
	};
} // namespace rawrbox
