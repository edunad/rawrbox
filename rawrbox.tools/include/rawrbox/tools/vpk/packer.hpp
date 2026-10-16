#pragma once

#include <rawrbox/utils/logger.hpp>

#include <glaze/glaze.hpp>

#include <filesystem>
#include <memory>
#include <mutex>
#include <span>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

namespace basisu {
	class image;
} // namespace basisu

namespace rawrbox {
	struct VPKPackSettings {
		std::vector<std::pair<std::string, std::filesystem::path>> inputs = {};
		std::filesystem::path output = {};
		std::filesystem::path cache = {};

		bool compress = false;

		std::vector<std::string> raw = {};
		std::vector<std::string> data = {}; // normal, roughness, masks, etc
		std::vector<std::string> preload = {};
	};

	enum class VPKPackType {
		COPY = 0,
		TEXTURE,
		MODEL,
		SCRIPT,

		COUNT
	};

	struct VPKPackFile {
		std::string path = {};
		std::filesystem::path source = {};
		rawrbox::VPKPackType type = rawrbox::VPKPackType::COPY;

		std::vector<std::pair<std::string, std::filesystem::path>> outputs = {};
		bool data = false;
		bool cached = false;
	};

	class VPKPacker {
	protected:
		rawrbox::VPKPackSettings _settings = {};
		std::vector<rawrbox::VPKPackFile> _files = {};
		std::mutex _modelLock;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-VPKPacker");
		// -------------

		virtual void collectFiles();

		// PROCESSING ----
		virtual void process(rawrbox::VPKPackFile& file);
		virtual void processTexture(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder);
		virtual void processModel(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder);
		virtual void processLUA(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder);
		virtual void processEmbeddedTextures(const rawrbox::VPKPackFile& file, const std::filesystem::path& glb);
		// ---------------

		[[nodiscard]] virtual bool decodeImage(std::span<const uint8_t> data, basisu::image& image) const;
		[[nodiscard]] virtual std::vector<uint8_t> encodeTexture(const std::string& name, const basisu::image& image, bool data) const;

		[[nodiscard]] virtual std::filesystem::path decodeURI(std::string_view uri) const;
		[[nodiscard]] virtual bool readModelJson(const std::filesystem::path& source, glz::generic_i64& json) const;
		[[nodiscard]] virtual std::vector<std::filesystem::path> getExternalFiles(glz::generic_i64& json, const std::filesystem::path& folder, bool images) const;
		[[nodiscard]] virtual std::unordered_set<int64_t> getDataImages(glz::generic_i64& json) const;
		[[nodiscard]] virtual std::string getCacheKey(const rawrbox::VPKPackFile& file) const;
		[[nodiscard]] virtual bool matches(const std::vector<std::string>& globs, const std::string& path) const;

	public:
		explicit VPKPacker(rawrbox::VPKPackSettings settings);
		VPKPacker(const VPKPacker&) = delete;
		VPKPacker(VPKPacker&&) = delete;
		VPKPacker& operator=(const VPKPacker&) = delete;
		VPKPacker& operator=(VPKPacker&&) = delete;
		virtual ~VPKPacker() = default;

		virtual void pack();
	};
} // namespace rawrbox