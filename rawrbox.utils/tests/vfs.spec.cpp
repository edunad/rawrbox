#include <rawrbox/utils/file.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk_writer.hpp>

#include <catch2/catch_test_macros.hpp>

#include <bit>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {
	std::vector<uint8_t> toData(const std::string& str) {
		return {str.begin(), str.end()};
	}

	void writeFile(const std::string& path, const std::string& data) {
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream stream(path, std::ios::binary | std::ios::trunc);
		stream.write(data.data(), static_cast<std::streamsize>(data.size()));
	}

	std::vector<std::string> toNames(const std::vector<std::filesystem::path>& paths, const std::string& root) {
		std::vector<std::string> names = {};
		for (const auto& path : paths) {
			names.push_back(rawrbox::VFS::normalize(path.lexically_relative(root)));
		}

		return names;
	}
} // namespace

TEST_CASE("VFS should behave as expected", "[rawrbox::VFS]") {
	const std::string folder = "./rawrbox-tests-vfs";
	const std::string assets = folder + "/assets";
	const std::string base = folder + "/base_dir.vpk";
	const std::string patch = folder + "/patch_dir.vpk";

	rawrbox::VFS::unmount(base);
	rawrbox::VFS::unmount(patch);
	std::filesystem::remove_all(folder);

	// PACKS ---
	writeFile(folder + "/base/textures/a.png", "packed a");
	writeFile(folder + "/base/textures/sub/b.png", "packed b");
	writeFile(folder + "/base/shaders/common.fxh", "packed shader");
	writeFile(folder + "/patch/textures/a.png", "patched a");

	rawrbox::VPKWriter baseWriter;
	baseWriter.add("textures/a.png", folder + "/base/textures/a.png");
	baseWriter.add("textures/sub/b.png", folder + "/base/textures/sub/b.png");
	baseWriter.add("shaders/common.fxh", folder + "/base/shaders/common.fxh", true);
	baseWriter.write(base);

	rawrbox::VPKWriter patchWriter;
	patchWriter.add("textures/a.png", folder + "/patch/textures/a.png");
	patchWriter.write(patch);
	// --------

	SECTION("rawrbox::VFS::normalize") {
		REQUIRE(rawrbox::VFS::normalize("./assets/Textures\\Foo.PNG") == "assets/textures/foo.png");
		REQUIRE(rawrbox::VFS::normalize("assets/./shaders/../textures/") == "assets/textures");
		REQUIRE(rawrbox::VFS::normalize("assets") == "assets");
		REQUIRE(rawrbox::VFS::normalize(".").empty());
	}

	SECTION("rawrbox::VFS::mount") {
		REQUIRE_FALSE(rawrbox::VFS::mount(folder + "/missing_dir.vpk", assets));

		REQUIRE(rawrbox::VFS::mount(base, assets));
		REQUIRE_FALSE(rawrbox::VFS::mount(base, assets));
		REQUIRE_FALSE(rawrbox::VFS::mount(folder + "/./base_dir.vpk", assets));

		REQUIRE(rawrbox::VFS::unmount(base));
		REQUIRE_FALSE(rawrbox::VFS::unmount(base));
	}

	SECTION("rawrbox::VFS::mount (root)") {
		writeFile(folder + "/root/a.txt", "root a");

		rawrbox::VPKWriter rootWriter;
		rootWriter.add("rawrbox-vfs-root/a.txt", folder + "/root/a.txt");
		rootWriter.write(folder + "/root_dir.vpk");

		REQUIRE(rawrbox::VFS::mount(folder + "/root_dir.vpk"));
		REQUIRE(rawrbox::VFS::contains("./rawrbox-vfs-root/a.txt"));
		REQUIRE(rawrbox::VFS::read("rawrbox-vfs-root/a.txt") == toData("root a"));
		REQUIRE(rawrbox::VFS::unmount(folder + "/root_dir.vpk"));
	}

	SECTION("rawrbox::VFS::read") {
		REQUIRE_FALSE(rawrbox::VFS::contains(assets + "/textures/a.png"));
		REQUIRE(rawrbox::VFS::mount(base, assets));

		REQUIRE(rawrbox::VFS::contains(assets + "/textures/a.png"));
		REQUIRE(rawrbox::VFS::contains(assets + "/TEXTURES/A.PNG"));
		REQUIRE(rawrbox::VFS::contains(assets + "/textures"));
		REQUIRE(rawrbox::VFS::contains(assets + "/textures/sub/"));
		REQUIRE(rawrbox::VFS::contains(assets));
		REQUIRE_FALSE(rawrbox::VFS::contains(assets + "/textures/su"));
		REQUIRE_FALSE(rawrbox::VFS::contains(assets + "/missing.png"));
		REQUIRE_FALSE(rawrbox::VFS::contains(folder + "/textures/a.png"));

		REQUIRE(rawrbox::VFS::read(assets + "/textures/a.png") == toData("packed a"));
		REQUIRE(rawrbox::VFS::read(assets + "/./textures/../shaders/common.fxh") == toData("packed shader"));
		REQUIRE(rawrbox::VFS::read(assets + "/missing.png").empty());
	}

	SECTION("rawrbox::VFS::read (priority)") {
		REQUIRE(rawrbox::VFS::mount(base, assets));
		REQUIRE(rawrbox::VFS::mount(patch, assets));
		REQUIRE(rawrbox::VFS::read(assets + "/textures/a.png") == toData("patched a"));
		REQUIRE(rawrbox::VFS::read(assets + "/textures/sub/b.png") == toData("packed b"));

		writeFile(assets + "/textures/a.png", "loose a");
		REQUIRE(rawrbox::VFS::read(assets + "/textures/a.png") == toData("loose a"));

		std::filesystem::remove(assets + "/textures/a.png");
		REQUIRE(rawrbox::VFS::unmount(patch));
		REQUIRE(rawrbox::VFS::read(assets + "/textures/a.png") == toData("packed a"));
	}

	SECTION("rawrbox::VFS::open") {
		REQUIRE(rawrbox::VFS::mount(base, assets));

		auto packed = rawrbox::VFS::open(assets + "/textures/sub/b.png");
		REQUIRE(packed != nullptr);
		REQUIRE(packed->size() == 8);

		std::string data(3, '\0');
		REQUIRE(packed->read(2, std::bit_cast<uint8_t*>(data.data()), data.size()) == 3);
		REQUIRE(data == "cke");
		REQUIRE(packed->read(6, std::bit_cast<uint8_t*>(data.data()), data.size()) == 2);

		writeFile(assets + "/loose.txt", "loose file");
		auto loose = rawrbox::VFS::open(assets + "/loose.txt");
		REQUIRE(loose != nullptr);
		REQUIRE(loose->size() == 10);
		REQUIRE(loose->read(6, std::bit_cast<uint8_t*>(data.data()), data.size()) == 3);
		REQUIRE(data == "fil");

		REQUIRE(rawrbox::VFS::open(assets + "/missing.png") == nullptr);

		REQUIRE(rawrbox::VFS::unmount(base));
		REQUIRE(packed->read(0, std::bit_cast<uint8_t*>(data.data()), data.size()) == 3);
		REQUIRE(data == "pac");
	}

	SECTION("rawrbox::VFS::list") {
		REQUIRE(rawrbox::VFS::mount(base, assets));
		writeFile(assets + "/textures/loose.png", "loose");

		REQUIRE(toNames(rawrbox::VFS::list(assets + "/textures"), assets) == std::vector<std::string>{"textures/a.png", "textures/loose.png"});
		REQUIRE(toNames(rawrbox::VFS::list(assets + "/textures", true), assets) == std::vector<std::string>{"textures/a.png", "textures/loose.png", "textures/sub/b.png"});
		REQUIRE(toNames(rawrbox::VFS::list(assets, true), assets).size() == 4);
		REQUIRE(rawrbox::VFS::list(assets + "/missing").empty());
	}

	SECTION("rawrbox::FileUtils::getRawData") {
		REQUIRE(rawrbox::VFS::mount(base, assets));

		const auto data = rawrbox::FileUtils::getRawData(assets + "/textures/a.png");
		REQUIRE(data.size() == 9);
		REQUIRE(data.back() == '\0');
		REQUIRE(std::string(data.begin(), data.end() - 1) == "packed a");

		REQUIRE(rawrbox::FileUtils::getRawData(assets + "/missing.png").empty());
	}

	// CLEANUP -----
	rawrbox::VFS::unmount(base);
	rawrbox::VFS::unmount(patch);

	std::filesystem::remove_all(folder);
	// -------------
}