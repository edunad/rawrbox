#pragma once
#include <filesystem>
#include <string>
#include <vector>

namespace rawrbox {
	enum LoadStatus {
		NONE = 0,
		UNLOADED,
		LOADING,
		LOADED
	};

	class Resource {
	public:
		rawrbox::LoadStatus status = rawrbox::LoadStatus::NONE;

		uint32_t flags = 0; // Used for certain files
		uint32_t crc32 = 0;

		std::filesystem::path filePath;
		std::string extention;

		virtual bool load(const std::vector<uint8_t>& buffer) { return true; };
		virtual void upload(){};

		Resource() = default;
		virtual ~Resource() = default;
	};
} // namespace rawrbox
