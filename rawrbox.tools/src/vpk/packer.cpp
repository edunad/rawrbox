#include <rawrbox/tools/vpk/packer.hpp>
#include <rawrbox/utils/file.hpp>
#include <rawrbox/utils/md5.hpp>
#include <rawrbox/utils/path.hpp>
#include <rawrbox/utils/string.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk_writer.hpp>

#ifdef RAWRBOX_SCRIPTING
	#include <rawrbox/scripting/utils/lua.hpp>
#endif

#include <BS_thread_pool.hpp>

#include <encoder/basisu_comp.h>
#include <fmt/color.h>
#include <gltfpack.h>
#include <meshoptimizer.h>
#include <webp/decode.h>

#include <algorithm>
#include <array>
#include <bit>
#include <cctype>
#include <chrono>
#include <future>
#include <map>
#include <set>
#include <thread>
#include <unordered_map>

// GLTFPACK (gltf/gltfpack.cpp) ---
Settings defaults();
int gltfpack(const char* input, const char* output, const char* report, Settings settings);
// ----

namespace rawrbox {
	VPKPacker::VPKPacker(rawrbox::VPKPackSettings settings) : _settings(std::move(settings)) {
		if (this->_settings.inputs.empty()) RAWRBOX_CRITICAL("Missing input folders");
		for (const auto& input : this->_settings.inputs) {
			if (!std::filesystem::is_directory(input.second)) RAWRBOX_CRITICAL("Input folder '{}' not found", input.second.generic_string());
		}

		if (this->_settings.output.empty()) RAWRBOX_CRITICAL("Missing output VPK");
		if (this->_settings.cache.empty()) RAWRBOX_CRITICAL("Missing cache folder"); // Used for CMAKE development

		for (auto* globs : {&this->_settings.raw, &this->_settings.data, &this->_settings.preload}) {
			for (auto& glob : *globs) {
				glob = rawrbox::StrUtils::toLower(glob);
			}
		}

		basisu::basisu_encoder_init();
	}

	void VPKPacker::collectFiles() {
		std::unordered_map<std::string, size_t> indexes = {};

		for (const auto& input : this->_settings.inputs) {
			for (const auto& entry : std::filesystem::recursive_directory_iterator(input.second)) {
				if (!entry.is_regular_file()) continue;

				rawrbox::VPKPackFile file = {};
				file.path = rawrbox::VFS::normalize(std::filesystem::path(input.first) / entry.path().lexically_relative(input.second));
				file.source = entry.path();
				file.data = this->matches(this->_settings.data, file.path);

				const auto extension = std::filesystem::path(file.path).extension();
				if (!this->matches(this->_settings.raw, file.path)) {
					if ((extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".tga") && file.path.find(".vertex.") == std::string::npos) file.type = rawrbox::VPKPackType::TEXTURE;
					if (extension == ".gltf" || extension == ".glb") file.type = rawrbox::VPKPackType::MODEL;
#ifdef RAWRBOX_SCRIPTING
					if (extension == ".lua" || extension == ".luau") file.type = rawrbox::VPKPackType::SCRIPT;
#endif
				}

				// Duplicated files ---
				auto index = indexes.find(file.path);
				if (index != indexes.end()) {
					this->_logger->warn("'{}' already packed from '{}', skipping '{}'", file.path, this->_files[index->second].source.generic_string(), file.source.generic_string());
					continue;
				}
				// -------------------------

				indexes[file.path] = this->_files.size();
				this->_files.push_back(file);
			}
		}

		// DATA TEXTURES (normal, occlusion, metallic, roughness, etc) ---
		for (const auto& file : this->_files) {
			const auto extension = std::filesystem::path(file.path).extension();
			if (extension != ".gltf" && extension != ".glb") continue;

			glz::generic_i64 json = {};
			if (!this->readModelJson(file.source, json)) {
				this->_logger->warn("Failed to parse '{}', its textures will be packed as color", file.path);
				continue;
			}

			if (!json.contains("images")) continue;

			auto& images = json["images"].get_array();
			const auto folder = std::filesystem::path(file.path).parent_path();

			for (const auto index : this->getDataImages(json)) {
				if (index < 0 || static_cast<size_t>(index) >= images.size() || !images[static_cast<size_t>(index)].contains("uri")) continue;

				const auto& uri = images[static_cast<size_t>(index)]["uri"].get<std::string>();
				if (uri.starts_with("data:")) continue;

				auto found = indexes.find(rawrbox::VFS::normalize(folder / this->decodeURI(uri)));
				if (found != indexes.end()) this->_files[found->second].data = true;
			}
		}
		// -------------
	}

	void VPKPacker::process(rawrbox::VPKPackFile& file) {
		const auto key = this->getCacheKey(file);
		const auto folder = this->_settings.cache / key;

		if (std::filesystem::exists(folder)) {
			file.cached = true;
		} else {
			const auto temp = this->_settings.cache / fmt::format("{}.{}.tmp", key, rawrbox::MD5::toString(rawrbox::MD5::calculate(std::bit_cast<const uint8_t*>(file.path.data()), file.path.size())));
			std::filesystem::remove_all(temp);
			std::filesystem::create_directories(temp);

			this->_logger->info("Processing '{}'", fmt::styled(file.path, fmt::fg(fmt::color::coral)));

			switch (file.type) {
				case rawrbox::VPKPackType::TEXTURE:
					this->processTexture(file, temp);
					break;
				case rawrbox::VPKPackType::SCRIPT:
					this->processLUA(file, temp);
					break;
				default:
					this->processModel(file, temp);
					break;
			}

			std::error_code err;
			for (int attempt = 0; attempt < 20; attempt++) {
				std::filesystem::rename(temp, folder, err);
				if (!err || std::filesystem::exists(folder)) break;

				std::this_thread::sleep_for(std::chrono::milliseconds(50));
			}

			if (err) {
				std::filesystem::remove_all(temp);
				if (!std::filesystem::exists(folder)) RAWRBOX_CRITICAL("Failed to cache '{}' ──> {}", file.path, err.message());
			}
		}

		const auto parent = std::filesystem::path(file.path).parent_path();
		for (const auto& entry : std::filesystem::directory_iterator(folder)) {
			file.outputs.emplace_back(rawrbox::VFS::normalize(parent / entry.path().filename()), entry.path());
		}
	}

	void VPKPacker::processLUA(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder) {
#ifdef RAWRBOX_SCRIPTING
		const auto source = rawrbox::VFS::read(file.source);

		std::string bytecode = {};
		try {
			bytecode = rawrbox::LuaUtils::precompile(std::string(source.begin(), source.end()));
		} catch (const std::exception& err) {
			RAWRBOX_CRITICAL("Failed to compile script '{}' ──> {}", file.path, err.what());
		}

		if (!rawrbox::FileUtils::saveData(folder / std::filesystem::path(file.path).filename(), std::vector<uint8_t>(bytecode.begin(), bytecode.end()))) RAWRBOX_CRITICAL("Failed to write script '{}'", file.path);
#else
		RAWRBOX_CRITICAL("Failed to compile script '{}', RAWRBOX.SCRIPTING is disabled", file.path);
#endif
	}

	void VPKPacker::processTexture(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder) {
		basisu::image image = {};
		if (!this->decodeImage(rawrbox::VFS::read(file.source), image)) RAWRBOX_CRITICAL("Failed to decode texture '{}'", file.path);

		const auto ktx2 = this->encodeTexture(file.path, image, file.data);
		if (!rawrbox::FileUtils::saveData(folder / std::filesystem::path(file.path).filename(), ktx2)) RAWRBOX_CRITICAL("Failed to write texture '{}'", file.path);
	}

	void VPKPacker::processModel(const rawrbox::VPKPackFile& file, const std::filesystem::path& folder) {
		const auto output = folder / std::filesystem::path(file.path).filename();

		Settings settings = defaults();
		settings.keep_nodes = true;
		settings.keep_materials = true;
		settings.keep_extras = true;
		settings.keep_attributes = true;
		settings.anim_freq = 0;
		settings.quantize = this->_settings.compress;
		settings.compress = true;
		settings.texture_ref = output.extension() == ".gltf";

		// THREAD ----
		{
			std::scoped_lock lock(this->_modelLock);
			if (gltfpack(file.source.string().c_str(), output.string().c_str(), nullptr, settings) != 0 || !std::filesystem::exists(output))
				RAWRBOX_CRITICAL("Failed to process model '{}'", file.path);
		}
		// -----------

		if (output.extension() == ".glb")
			this->processEmbeddedTextures(file, output);
	}

	void VPKPacker::processEmbeddedTextures(const rawrbox::VPKPackFile& file, const std::filesystem::path& glb) {
		auto data = rawrbox::VFS::read(glb);

		// https://registry.khronos.org/glTF/specs/2.0/glTF-2.0.html#binary-gltf-layout
		const auto readU32 = [&data](size_t offset) -> uint32_t {
			uint32_t value = 0;
			std::memcpy(&value, data.data() + offset, sizeof(uint32_t));
			return value;
		};

		// CHUNKS ---
		if (data.size() < 28 || std::memcmp(data.data(), "glTF", 4) != 0 || readU32(16) != 0x4E4F534A) // Bad magic
			RAWRBOX_CRITICAL("Invalid GLB '{}'", file.path);

		const uint32_t jsonLength = readU32(12);
		const size_t binOffset = 20 + static_cast<size_t>(jsonLength);
		if (binOffset + 8 > data.size() || readU32(binOffset + 4) != 0x004E4942) return;

		const uint32_t binLength = readU32(binOffset);
		if (binOffset + 8 + binLength > data.size())
			RAWRBOX_CRITICAL("Invalid GLB '{}', binary chunk is compressed", file.path);

		const std::span<const uint8_t> bin(data.data() + binOffset + 8, binLength);

		glz::generic_i64 json = {};
		if (glz::read_json(json, std::string_view(std::bit_cast<const char*>(data.data() + 20), jsonLength))) RAWRBOX_CRITICAL("Invalid GLB '{}', failed to parse json", file.path);
		if (!json.contains("images") || !json.contains("bufferViews") || !json.contains("buffers") || json["buffers"].get_array().empty() || json["buffers"][0].contains("uri")) return;

		const auto slice = [&bin, &file](const glz::generic_i64& range) -> std::span<const uint8_t> {
			const auto offset = range.contains("byteOffset") ? static_cast<uint64_t>(range["byteOffset"].as<int64_t>()) : 0;
			const auto length = static_cast<uint64_t>(range["byteLength"].as<int64_t>());
			if (offset + length > bin.size())
				RAWRBOX_CRITICAL("Invalid GLB '{}', buffer view outside of the binary chunk", file.path);

			return bin.subspan(static_cast<size_t>(offset), static_cast<size_t>(length));
		};
		// --------------

		// ENCODE TEXTURES ---
		const auto dataImages = this->getDataImages(json);

		auto& views = json["bufferViews"].get_array();
		auto& images = json["images"].get_array();

		std::unordered_map<size_t, std::vector<uint8_t>> encoded = {};
		for (size_t i = 0; i < images.size(); i++) {
			auto& image = images[i];
			if (!image.contains("bufferView")) continue;

			const auto viewIndex = static_cast<size_t>(image["bufferView"].as<int64_t>());

			basisu::image decoded = {};
			if (!this->decodeImage(slice(views.at(viewIndex)), decoded)) continue;

			encoded[viewIndex] = this->encodeTexture(fmt::format("{} (image {})", file.path, i), decoded, dataImages.contains(static_cast<int64_t>(i)));
			image["mimeType"] = "image/ktx2";
		}

		if (encoded.empty()) return;
		// --------------

		// REBUILD BINARY ---
		std::vector<uint8_t> newBin = {};
		const auto append = [&newBin](std::span<const uint8_t> bytes) -> int64_t {
			newBin.resize((newBin.size() + 3) & ~static_cast<size_t>(3), 0);

			const auto offset = static_cast<int64_t>(newBin.size());
			newBin.insert(newBin.end(), bytes.begin(), bytes.end());

			return offset;
		};

		for (size_t i = 0; i < views.size(); i++) {
			auto& view = views[i];

			if (view.contains("buffer") && view["buffer"].as<int64_t>() == 0) {
				auto replacement = encoded.find(i);
				if (replacement != encoded.end()) {
					view["byteOffset"] = append(replacement->second);
					view["byteLength"] = static_cast<int64_t>(replacement->second.size());
				} else {
					view["byteOffset"] = append(slice(view));
				}
			}

			if (!view.contains("extensions")) continue;
			for (const auto* extension : {"EXT_meshopt_compression", "KHR_meshopt_compression"}) {
				if (!view["extensions"].contains(extension)) continue;

				auto& compression = view["extensions"][extension];
				if (compression["buffer"].as<int64_t>() != 0) continue;

				compression["byteOffset"] = append(slice(compression));
			}
		}

		newBin.resize((newBin.size() + 3) & ~static_cast<size_t>(3), 0);
		json["buffers"][0]["byteLength"] = static_cast<int64_t>(newBin.size());
		// --------------

		// WRITE ---
		auto jsonText = glz::write_json(json);
		if (!jsonText) RAWRBOX_CRITICAL("Failed to write GLB '{}' json", file.path);

		auto& text = jsonText.value();
		text.resize((text.size() + 3) & ~static_cast<size_t>(3), ' ');

		std::vector<uint8_t> out(28 + text.size() + newBin.size());

		const std::array<uint32_t, 4> header = {2, static_cast<uint32_t>(out.size()), static_cast<uint32_t>(text.size()), 0x4E4F534A}; // version, length, json length, JSON
		const std::array<uint32_t, 2> binHeader = {static_cast<uint32_t>(newBin.size()), 0x004E4942};                                  // bin length, BIN

		std::memcpy(out.data(), "glTF", 4);
		std::memcpy(out.data() + 4, header.data(), sizeof(header));
		std::memcpy(out.data() + 20, text.data(), text.size());
		std::memcpy(out.data() + 20 + text.size(), binHeader.data(), sizeof(binHeader));
		std::memcpy(out.data() + 28 + text.size(), newBin.data(), newBin.size());

		if (!rawrbox::FileUtils::saveData(glb, out)) RAWRBOX_CRITICAL("Failed to write GLB '{}'", file.path);
		// --------------

		this->_logger->info("Converted {} embedded textures on '{}'", encoded.size(), file.path);
	}

	bool VPKPacker::decodeImage(std::span<const uint8_t> data, basisu::image& image) const {
		if (data.size() > 3 && data[0] == 0x89 && data[1] == 'P' && data[2] == 'N' && data[3] == 'G') return basisu::load_png(data.data(), data.size(), image);
		if (data.size() > 2 && data[0] == 0xFF && data[1] == 0xD8 && data[2] == 0xFF) return basisu::load_jpg(data.data(), data.size(), image);

		// WEBP ---
		if (data.size() > 11 && std::memcmp(data.data(), "RIFF", 4) == 0 && std::memcmp(data.data() + 8, "WEBP", 4) == 0) {
			int width = 0;
			int height = 0;

			const std::unique_ptr<uint8_t, decltype(&WebPFree)> pixels(WebPDecodeRGBA(data.data(), data.size(), &width, &height), &WebPFree);
			if (pixels == nullptr) return false;

			image = basisu::image(pixels.get(), static_cast<uint32_t>(width), static_cast<uint32_t>(height), 4);
			return true;
		}
		// --------------

		// TGA ---
		int width = 0;
		int height = 0;
		int channels = 0;

		const std::unique_ptr<uint8_t, decltype(&std::free)> pixels(basisu::read_tga(data.data(), static_cast<uint32_t>(data.size()), width, height, channels), &std::free);
		if (pixels == nullptr) return false;

		image = basisu::image(pixels.get(), static_cast<uint32_t>(width), static_cast<uint32_t>(height), static_cast<uint32_t>(channels));
		return true;
		// --------------
	}

	std::vector<uint8_t> VPKPacker::encodeTexture(const std::string& name, const basisu::image& image, bool data) const {
		basisu::job_pool pool(std::max(1U, std::thread::hardware_concurrency()));

		basisu::basis_compressor_params params = {};
		params.set_format_mode(this->_settings.compress ? basist::basis_tex_format::cETC1S : basist::basis_tex_format::cUASTC_LDR_4x4);
		params.set_srgb_options(!data);
		params.set_ldr_srgb_channel_weights(!data);
		params.m_source_images.push_back(image);
		params.m_pJob_pool = &pool;
		params.m_multithreading = true;
		params.m_create_ktx2_file = true;
		params.m_ktx2_uastc_supercompression = basist::KTX2_SS_ZSTANDARD;
		params.m_write_output_basis_or_ktx2_files = false;
		params.m_status_output = false;
		params.m_mip_gen = true;
		params.m_mip_filter = "box";

		basisu::basis_compressor compressor = {};
		if (!compressor.init(params) || compressor.process() != basisu::basis_compressor::cECSuccess) RAWRBOX_CRITICAL("Failed to encode texture '{}'", name);

		const auto& ktx2 = compressor.get_output_ktx2_file();
		return {ktx2.begin(), ktx2.end()};
	}

	bool VPKPacker::readModelJson(const std::filesystem::path& source, glz::generic_i64& json) const {
		const auto data = rawrbox::VFS::read(source);
		std::string_view text(std::bit_cast<const char*>(data.data()), data.size());

		// GLB, only the json chunk ---
		if (data.size() >= 20 && std::memcmp(data.data(), "glTF", 4) == 0) {
			uint32_t length = 0;
			std::memcpy(&length, data.data() + 12, sizeof(uint32_t));
			if (20 + static_cast<size_t>(length) > data.size()) return false;

			text = text.substr(20, length);
		}
		// --------------

		return !glz::read_json(json, text);
	}

	std::filesystem::path VPKPacker::decodeURI(std::string_view uri) const {
		std::string decoded = {};
		decoded.reserve(uri.size());

		for (size_t i = 0; i < uri.size(); i++) {
			if (uri[i] == '%' && i + 2 < uri.size() && std::isxdigit(static_cast<unsigned char>(uri[i + 1])) != 0 && std::isxdigit(static_cast<unsigned char>(uri[i + 2])) != 0) {
				decoded.push_back(static_cast<char>(std::stoi(std::string(uri.substr(i + 1, 2)), nullptr, 16)));
				i += 2;
				continue;
			}

			decoded.push_back(uri[i]);
		}

		return std::filesystem::path(std::u8string(decoded.begin(), decoded.end()));
	}

	std::vector<std::filesystem::path> VPKPacker::getExternalFiles(glz::generic_i64& json, const std::filesystem::path& folder, bool images) const {
		std::vector<std::filesystem::path> files = {};

		for (const auto* category : {"buffers", "images"}) {
			if (!json.contains(category) || (!images && std::string_view(category) == "images")) continue;

			for (auto& entry : json[category].get_array()) {
				if (!entry.contains("uri")) continue;

				const auto& uri = entry["uri"].get<std::string>();
				if (!uri.starts_with("data:")) files.push_back(folder / this->decodeURI(uri));
			}
		}

		return files;
	}

	std::unordered_set<int64_t> VPKPacker::getDataImages(glz::generic_i64& json) const {
		std::unordered_set<int64_t> images = {};
		if (!json.contains("materials") || !json.contains("textures")) return images;

		auto& textures = json["textures"].get_array();
		for (auto& material : json["materials"].get_array()) {
			std::vector<glz::generic_i64*> references = {};
			if (material.contains("normalTexture"))
				references.push_back(&material["normalTexture"]);

			if (material.contains("occlusionTexture"))
				references.push_back(&material["occlusionTexture"]);

			if (material.contains("pbrMetallicRoughness") && material["pbrMetallicRoughness"].contains("metallicRoughnessTexture"))
				references.push_back(&material["pbrMetallicRoughness"]["metallicRoughnessTexture"]);

			for (auto* reference : references) {
				auto& texture = textures.at(static_cast<size_t>((*reference)["index"].as<int64_t>()));
				if (texture.contains("source")) images.insert(texture["source"].as<int64_t>());

				if (!texture.contains("extensions")) continue;
				for (const auto* extension : {"EXT_texture_webp", "KHR_texture_basisu"}) {
					if (texture["extensions"].contains(extension))
						images.insert(texture["extensions"][extension]["source"].as<int64_t>());
				}
			}
		}

		return images;
	}

	std::string VPKPacker::getCacheKey(const rawrbox::VPKPackFile& file) const {
		rawrbox::MD5 md5 = {};

		auto options = fmt::format("v5|{}|{}|{}|{}|{}|{}", BASISU_LIB_VERSION, MESHOPTIMIZER_VERSION, std::filesystem::path(file.path).filename().generic_string(), static_cast<int>(file.type), this->_settings.compress, file.data);

#ifdef RAWRBOX_SCRIPTING
		if (file.type == rawrbox::VPKPackType::SCRIPT) {
	#ifdef _DEBUG
			options += "|luau-" RAWRBOX_LUAU_VERSION "|debug";
	#else
			options += "|luau-" RAWRBOX_LUAU_VERSION;
	#endif
		}
#endif
		md5.update(std::bit_cast<const uint8_t*>(options.data()), options.size());

		const auto data = rawrbox::VFS::read(file.source);
		md5.update(data.data(), data.size());

		// External files ---
		glz::generic_i64 json = {};
		if (file.type == rawrbox::VPKPackType::MODEL && this->readModelJson(file.source, json)) {
			for (const auto& external : this->getExternalFiles(json, file.source.parent_path(), std::filesystem::path(file.path).extension() == ".glb")) {
				const auto externalData = rawrbox::VFS::read(external);
				md5.update(externalData.data(), externalData.size());
			}
		}
		// --------------------

		return rawrbox::MD5::toString(md5.finalize());
	}

	bool VPKPacker::matches(const std::vector<std::string>& globs, const std::string& path) const {
		return std::ranges::any_of(globs, [&path](const auto& glob) { return rawrbox::PathUtils::matchGlob(glob, path); });
	}

	void VPKPacker::pack() {
		this->collectFiles();
		std::filesystem::create_directories(this->_settings.cache);

		// PROCESS ---
		{
			BS::thread_pool<> pool(std::max(1U, std::thread::hardware_concurrency()));
			std::vector<std::future<void>> jobs = {};

			for (auto& file : this->_files) {
				if (file.type == rawrbox::VPKPackType::COPY) continue;
				jobs.push_back(pool.submit_task([this, &file]() { this->process(file); }));
			}

			for (auto& job : jobs) {
				job.get();
			}
		}
		// -----------

		// CLEANUP CACHE ---
		std::set<std::filesystem::path> used = {};
		for (const auto& file : this->_files) {
			for (const auto& output : file.outputs) {
				used.insert(output.second.parent_path());
			}
		}

		for (const auto& entry : std::filesystem::directory_iterator(this->_settings.cache)) {
			const auto name = entry.path().filename().string();
			if (name.size() < 32 || !std::all_of(name.begin(), name.begin() + 32, [](char c) { return std::isxdigit(static_cast<unsigned char>(c)) != 0; })) continue;
			if (!used.contains(entry.path())) std::filesystem::remove_all(entry.path());
		}
		// -----------

		// ENTRIES ---
		std::map<std::string, std::filesystem::path> entries = {};
		for (const auto& file : this->_files) {
			if (file.type == rawrbox::VPKPackType::COPY) entries.emplace(file.path, file.source);
		}

		for (const auto& file : this->_files) {
			for (const auto& output : file.outputs) {
				entries.insert_or_assign(output.first, output.second);
			}
		}
		// -----------

		// WRITE ---
		rawrbox::VPKWriter writer;
		for (const auto& entry : entries) {
			const auto extension = std::filesystem::path(entry.first).extension();
			const bool streamed = extension == ".ogg" || extension == ".mp3" || extension == ".wav" || extension == ".flac" || extension == ".webm"; // Can't be compressed

			writer.add(entry.first, entry.second, this->matches(this->_settings.preload, entry.first), !streamed);
		}

		writer.write(this->_settings.output);
		// -----------

		const auto processed = std::ranges::count_if(this->_files, [](const auto& file) {
			return file.type != rawrbox::VPKPackType::COPY;
		});

		const auto cached = std::ranges::count_if(this->_files, [](const auto& file) {
			return file.cached;
		});

		this->_logger->info("Packed {} files, {} converted ({} encoded, {} from cache)", entries.size(), processed, processed - cached, cached);
	}
} // namespace rawrbox