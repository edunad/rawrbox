#include <rawrbox/resources/loaders/json.hpp>
#include <rawrbox/resources/manager.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk_writer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <future>
#include <string>
#include <vector>

namespace {
	void writeFile(const std::string& path, const std::string& data) {
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream stream(path, std::ios::binary | std::ios::trunc);
		stream.write(data.data(), static_cast<std::streamsize>(data.size()));
	}
} // namespace

TEST_CASE("RESOURCES should load from a mounted VPK", "[rawrbox::RESOURCES]") {
	const std::string folder = "./rawrbox-tests-resources-vfs";
	const std::string assets = folder + "/assets";
	const std::string pack = folder + "/pack_dir.vpk";

	rawrbox::VFS::unmount(pack);
	std::filesystem::remove_all(folder);

	writeFile(folder + "/source/json/packed.json", R"({"source": "packed"})");
	writeFile(folder + "/source/json/folder/a.json", R"({"id": "a"})");
	writeFile(folder + "/source/json/folder/b.json", R"({"id": "b"})");
	writeFile(folder + "/source/json/shared.json", R"({"id": "shared"})");

	rawrbox::VPKWriter writer;
	writer.add("json/packed.json", folder + "/source/json/packed.json");
	writer.add("json/folder/a.json", folder + "/source/json/folder/a.json");
	writer.add("json/folder/b.json", folder + "/source/json/folder/b.json");
	writer.add("json/shared.json", folder + "/source/json/shared.json");
	writer.write(pack);

	REQUIRE(rawrbox::VFS::mount(pack, assets));

	SECTION("rawrbox::RESOURCES::loadFile") {
		auto* json = rawrbox::RESOURCES::loadFile<rawrbox::ResourceJSON>(assets + "/json/packed.json");
		REQUIRE(json != nullptr);
		REQUIRE(json->get()["source"].get<std::string>() == "packed");

		REQUIRE(rawrbox::RESOURCES::isLoaded(assets + "/json/packed.json"));
		REQUIRE(rawrbox::RESOURCES::isLoaded(assets + "/./JSON/Packed.json"));
		REQUIRE_FALSE(rawrbox::RESOURCES::isLoaded(assets + "/json/missing.json"));
	}

	SECTION("rawrbox::RESOURCES::loadFolder") {
		size_t loaded = 0;
		rawrbox::RESOURCES::loadFolder(assets + "/json/folder", nullptr, [&loaded](const std::string& /*file*/) { loaded++; });

		REQUIRE(loaded == 2);
		REQUIRE(rawrbox::RESOURCES::isLoaded(assets + "/json/folder/a.json"));
		REQUIRE(rawrbox::RESOURCES::isLoaded(assets + "/json/folder/b.json"));
	}

	SECTION("rawrbox::RESOURCES::loadFile same file from many threads") {
		std::vector<std::future<rawrbox::ResourceJSON*>> jobs = {};
		for (size_t i = 0; i < 8; i++) {
			jobs.push_back(std::async(std::launch::async, [&assets]() {
				return rawrbox::RESOURCES::loadFile<rawrbox::ResourceJSON>(assets + "/json/shared.json");
			}));
		}

		std::vector<rawrbox::ResourceJSON*> results = {};
		for (auto& job : jobs) {
			results.push_back(job.get());
		}

		REQUIRE(results.front() != nullptr);
		for (auto* result : results) {
			REQUIRE(result == results.front());
		}
	}

	// CLEANUP ----
	rawrbox::VFS::unmount(pack);
	std::filesystem::remove_all(folder);
	// -------------
}