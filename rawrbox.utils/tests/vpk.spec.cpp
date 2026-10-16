#include <rawrbox/utils/crc.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk.hpp>
#include <rawrbox/utils/vpk_writer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <random>
#include <string>
#include <vector>

namespace {
	std::vector<uint8_t> makeData(size_t size, uint8_t seed) {
		std::vector<uint8_t> data(size);
		for (size_t i = 0; i < size; i++) {
			data[i] = static_cast<uint8_t>((i * 131) + (i >> 8) + seed);
		}

		return data;
	}

	void writeFile(const std::string& path, const std::vector<uint8_t>& data) {
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream stream(path, std::ios::binary | std::ios::trunc);
		stream.write(std::bit_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	}

	void corruptFile(const std::string& path, uint64_t offset) {
		std::fstream stream(path, std::ios::binary | std::ios::in | std::ios::out);

		char byte = 0;
		stream.seekg(static_cast<std::streamoff>(offset));
		stream.read(&byte, 1);

		byte = static_cast<char>(~byte);
		stream.seekp(static_cast<std::streamoff>(offset));
		stream.write(&byte, 1);
	}
} // namespace

TEST_CASE("VPK should behave as expected", "[rawrbox::VPK]") {
	const std::string folder = "./rawrbox-tests-vpk";
	std::filesystem::remove_all(folder);

	const std::string pack = folder + "/pack/test_dir.vpk";
	const std::map<std::string, std::vector<uint8_t>> files = {
	    {"readme.txt", makeData(5, 1)},
	    {"textures/wall.png", makeData(300000, 2)},
	    {"textures/floor.png", makeData(70000, 3)},
	    {"shaders/include/common.fxh", makeData(2000, 4)},
	    {"fonts/debug.ttf", makeData(100000, 5)},
	    {"noextension", makeData(10, 6)},
	    {"empty.txt", {}},
	    {"Upper/Case.TXT", makeData(42, 7)},
	    {"models/house.glb", makeData(600000, 8)},
	};

	// PACK ---
	rawrbox::VPKWriter writer(256 * 1024, 64 * 1024);
	for (const auto& file : files) {
		writeFile(folder + "/source/" + file.first, file.second);
		writer.add(file.first, folder + "/source/" + file.first, file.first.starts_with("shaders/") || file.first.starts_with("fonts/"));
	}

	writer.write(pack);
	// --------

	SECTION("rawrbox::VPKWriter::add") {
		REQUIRE_THROWS(writer.add("readme.txt", folder + "/source/readme.txt"));
		REQUIRE_THROWS(writer.add("./README.TXT", folder + "/source/readme.txt"));
		REQUIRE_THROWS(writer.add("../outside.txt", folder + "/source/readme.txt"));
		REQUIRE_THROWS(writer.add("", folder + "/source/readme.txt"));
		REQUIRE_THROWS(writer.add("..", folder + "/source/readme.txt"));
		REQUIRE_THROWS(writer.add("trailing.", folder + "/source/readme.txt"));
		REQUIRE_THROWS(rawrbox::VPKWriter(5ULL * 1024 * 1024 * 1024));
	}

	SECTION("rawrbox::VPKWriter::write") {
		REQUIRE(std::filesystem::exists(pack));
		REQUIRE(std::filesystem::exists(folder + "/pack/test_000.vpk"));
		REQUIRE(std::filesystem::exists(folder + "/pack/test_001.vpk"));
	}

	SECTION("rawrbox::VPK::read") {
		rawrbox::VPK vpk(pack);
		REQUIRE(vpk.getEntries().size() == files.size());

		for (const auto& file : files) {
			const auto* entry = vpk.getEntry(rawrbox::VFS::normalize(file.first));
			REQUIRE(entry != nullptr);

			const auto data = vpk.read(*entry);
			REQUIRE(data == file.second);
			REQUIRE(entry->crc == CRC::Calculate(file.second.data(), file.second.size(), CRC::CRC_32()));
		}

		REQUIRE(vpk.getEntry("missing.txt") == nullptr);
	}

	SECTION("rawrbox::VPK::read (layout)") {
		rawrbox::VPK vpk(pack);

		const auto* shader = vpk.getEntry("shaders/include/common.fxh");
		REQUIRE(shader->archive == 0x7FFF);
		REQUIRE(shader->preloadSize == 2000);
		REQUIRE(shader->size == 0);

		const auto* font = vpk.getEntry("fonts/debug.ttf");
		REQUIRE(font->archive == 0x7FFF);
		REQUIRE(font->preloadSize == 0);
		REQUIRE(font->size == 100000);

		const auto* house = vpk.getEntry("models/house.glb");
		REQUIRE(house->archive != 0x7FFF);
		REQUIRE(house->size == 600000);
	}

	SECTION("rawrbox::VPK::read (range)") {
		rawrbox::VPK vpk(pack);

		const auto* house = vpk.getEntry("models/house.glb");
		const auto& expected = files.at("models/house.glb");

		std::ifstream stream = {};
		std::vector<uint8_t> data(100);

		REQUIRE(vpk.read(*house, stream, 65500, data.data(), data.size()) == data.size());
		REQUIRE(std::equal(data.begin(), data.end(), expected.begin() + 65500));

		REQUIRE(vpk.read(*house, stream, expected.size() - 10, data.data(), data.size()) == 10);
		REQUIRE(vpk.read(*house, stream, expected.size(), data.data(), data.size()) == 0);
	}

	SECTION("rawrbox::VPKWriter::write (unchanged archives)") {
		const std::string archive = folder + "/pack/test_000.vpk";
		const auto oldTime = std::filesystem::last_write_time(archive) - std::chrono::hours(1);
		std::filesystem::last_write_time(archive, oldTime);

		rawrbox::VPKWriter rewriter(256 * 1024, 64 * 1024);
		for (const auto& file : files) {
			rewriter.add(file.first, folder + "/source/" + file.first, file.first.starts_with("shaders/") || file.first.starts_with("fonts/"));
		}

		rewriter.write(pack);
		REQUIRE(std::filesystem::last_write_time(archive) == oldTime);
	}

	SECTION("rawrbox::VPKWriter::write (stale archives)") {
		rawrbox::VPKWriter rewriter(256 * 1024, 64 * 1024);
		rewriter.add("readme.txt", folder + "/source/readme.txt");
		rewriter.write(pack);

		REQUIRE(std::filesystem::exists(folder + "/pack/test_000.vpk"));
		REQUIRE_FALSE(std::filesystem::exists(folder + "/pack/test_001.vpk"));

		rawrbox::VPK vpk(pack);
		REQUIRE(vpk.getEntries().size() == 1);
		REQUIRE(vpk.read(*vpk.getEntry("readme.txt")) == files.at("readme.txt"));
	}

	SECTION("rawrbox::VPK (corrupted tree)") {
		corruptFile(pack, sizeof(rawrbox::VPKHeader) + 2);
		REQUIRE_THROWS(rawrbox::VPK(pack));
	}

	SECTION("rawrbox::VPK (corrupted file data)") {
		rawrbox::VPKHeader header = {};
		std::ifstream stream(pack, std::ios::binary);
		stream.read(std::bit_cast<char*>(&header), sizeof(rawrbox::VPKHeader));
		stream.close();

		corruptFile(pack, sizeof(rawrbox::VPKHeader) + header.treeSize + 500);
		REQUIRE_THROWS(rawrbox::VPK(pack));
	}

	SECTION("rawrbox::VPK (corrupted checksums)") {
		corruptFile(pack, std::filesystem::file_size(pack) - sizeof(rawrbox::VPKOtherMD5) - 10);
		REQUIRE_THROWS(rawrbox::VPK(pack));
	}

	SECTION("rawrbox::VPK (corrupted archive)") {
		const rawrbox::VPKEntry house = *rawrbox::VPK(pack).getEntry("models/house.glb");
		corruptFile(rawrbox::VPK(pack).getArchivePath(house.archive).generic_string(), house.offset + 70000);

		rawrbox::VPK vpk(pack);
		REQUIRE_THROWS(vpk.read(*vpk.getEntry("models/house.glb")));
		REQUIRE(vpk.read(*vpk.getEntry("readme.txt")) == files.at("readme.txt"));

		std::ifstream stream = {};
		std::vector<uint8_t> data(100);
		REQUIRE(vpk.read(*vpk.getEntry("models/house.glb"), stream, 0, data.data(), data.size()) == data.size());
	}

	SECTION("rawrbox::VPK (invalid header)") {
		std::fstream stream(pack, std::ios::binary | std::ios::in | std::ios::out);
		constexpr uint32_t version = 1;

		stream.seekp(4);
		stream.write(std::bit_cast<const char*>(&version), sizeof(uint32_t));
		stream.close();

		REQUIRE_THROWS(rawrbox::VPK(pack));
	}

	SECTION("rawrbox::VPK (truncated)") {
		std::filesystem::resize_file(pack, std::filesystem::file_size(pack) - 20);
		REQUIRE_THROWS(rawrbox::VPK(pack));
	}

	SECTION("rawrbox::VPKWriter::write (compressed)") {
		const std::string text = std::string(20000, 'a') + "rawrbox";
		const std::vector<uint8_t> textData(text.begin(), text.end());

		std::mt19937 rng(1);
		std::vector<uint8_t> noise(20000);
		for (auto& byte : noise) {
			byte = static_cast<uint8_t>(rng());
		}

		writeFile(folder + "/compress/text.txt", textData);
		writeFile(folder + "/compress/copy.txt", textData);
		writeFile(folder + "/compress/noise.bin", noise);

		rawrbox::VPKWriter compressed;
		compressed.add("text.txt", folder + "/compress/text.txt", false, true);
		compressed.add("copy.txt", folder + "/compress/copy.txt", false, true);
		compressed.add("preload.txt", folder + "/compress/text.txt", true, true);
		compressed.add("noise.bin", folder + "/compress/noise.bin", false, true);
		compressed.write(folder + "/compress/test_dir.vpk");

		rawrbox::VPK vpk(folder + "/compress/test_dir.vpk");

		const auto* textEntry = vpk.getEntry("text.txt");
		REQUIRE(textEntry->compressed);
		REQUIRE(textEntry->size < textData.size() / 10);
		REQUIRE(vpk.read(*textEntry) == textData);
		REQUIRE(textEntry->crc == CRC::Calculate(textData.data(), textData.size(), CRC::CRC_32()));

		const auto* preloadEntry = vpk.getEntry("preload.txt");
		REQUIRE(preloadEntry->compressed);
		REQUIRE(preloadEntry->preloadSize > 0);
		REQUIRE(vpk.read(*preloadEntry) == textData);

		// Duplicated, points to the same data ---
		const auto* copyEntry = vpk.getEntry("copy.txt");
		REQUIRE(copyEntry->archive == textEntry->archive);
		REQUIRE(copyEntry->offset == textEntry->offset);
		REQUIRE(vpk.read(*copyEntry) == textData);

		// Skip compression ---
		REQUIRE_FALSE(vpk.getEntry("noise.bin")->compressed);
		REQUIRE(vpk.read(*vpk.getEntry("noise.bin")) == noise);

		std::ifstream stream = {};
		std::vector<uint8_t> part(10);

		REQUIRE_THROWS(vpk.read(*textEntry, stream, 0, part.data(), part.size()));

		// VFS streams ---
		REQUIRE(rawrbox::VFS::mount(folder + "/compress/test_dir.vpk", "rawrbox-tests-vpk-mount"));

		auto file = rawrbox::VFS::open("rawrbox-tests-vpk-mount/text.txt");
		REQUIRE(file != nullptr);
		REQUIRE(file->size() == textData.size());
		REQUIRE(file->read(textData.size() - 7, part.data(), part.size()) == 7);
		REQUIRE(std::string(part.begin(), part.begin() + 7) == "rawrbox");

		REQUIRE(rawrbox::VFS::unmount(folder + "/compress/test_dir.vpk"));
	}

	SECTION("rawrbox::VPK (invalid name)") {
		REQUIRE_THROWS(rawrbox::VPK(folder + "/pack/test_000.vpk"));
		REQUIRE_THROWS(rawrbox::VPK(folder + "/pack/missing_dir.vpk"));
	}

	// CLEANUP -----
	std::filesystem::remove_all(folder);
	// --------------
}