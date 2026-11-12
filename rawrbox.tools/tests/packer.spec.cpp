#include <rawrbox/tools/vpk/packer.hpp>
#include <rawrbox/utils/vfs.hpp>
#include <rawrbox/utils/vpk.hpp>

#include <catch2/catch_test_macros.hpp>

#include <array>
#include <bit>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

namespace rawrbox {
	class TestPacker : public rawrbox::VPKPacker {
	public:
		using rawrbox::VPKPacker::collectFiles;
		using rawrbox::VPKPacker::getCacheKey;
		using rawrbox::VPKPacker::VPKPacker;

		[[nodiscard]] const rawrbox::VPKPackFile& getFile(const std::string& path) const {
			for (const auto& file : this->_files) {
				if (file.path == path) return file;
			}

			throw std::runtime_error("Missing file " + path);
		}

		[[nodiscard]] const std::vector<rawrbox::VPKPackFile>& getFiles() const { return this->_files; }
	};

	static std::vector<uint8_t> readPackerFile(const std::string& path) {
		std::ifstream stream(path, std::ios::binary);
		return {std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
	}

	static void writePackerFile(const std::string& path, const std::vector<uint8_t>& data) {
		std::filesystem::create_directories(std::filesystem::path(path).parent_path());

		std::ofstream stream(path, std::ios::binary | std::ios::trunc);
		stream.write(std::bit_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
	}

	static void writePackerFile(const std::string& path, const std::string& data) {
		writePackerFile(path, std::vector<uint8_t>(data.begin(), data.end()));
	}

	// 4x4 uncompressed RGBA TGA
	static void writeTGA(const std::string& path, uint8_t seed) {
		std::vector<uint8_t> data = {0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 4, 0, 4, 0, 32, 0x28};
		for (uint8_t i = 0; i < 16; i++) {
			data.insert(data.end(), {static_cast<uint8_t>(seed + i * 10), static_cast<uint8_t>(seed + i * 5), static_cast<uint8_t>(i * 15), 255});
		}

		writePackerFile(path, data);
	}

	// Triangle GLB
	static void writeGLB(const std::string& path) {
		std::string json = R"({"asset":{"version":"2.0"},"scene":0,"scenes":[{"nodes":[0]}],"nodes":[{"mesh":0}],"meshes":[{"primitives":[{"attributes":{"POSITION":0}}]}],)"
				   R"("accessors":[{"bufferView":0,"componentType":5126,"count":3,"type":"VEC3","min":[0,0,0],"max":[1,1,0]}],"bufferViews":[{"buffer":0,"byteLength":36}],"buffers":[{"byteLength":36}]})";
		json.resize((json.size() + 3) & ~static_cast<size_t>(3), ' ');

		const std::vector<float> positions = {0, 0, 0, 1, 0, 0, 0, 1, 0};
		const auto binSize = static_cast<uint32_t>(positions.size() * sizeof(float));
		const auto jsonSize = static_cast<uint32_t>(json.size());
		const uint32_t total = 12 + 8 + jsonSize + 8 + binSize;

		std::vector<uint8_t> data(total);

		const std::array<uint32_t, 4> header = {2, total, jsonSize, 0x4E4F534A};
		const std::array<uint32_t, 2> binHeader = {binSize, 0x004E4942};

		std::memcpy(data.data(), "glTF", 4);
		std::memcpy(data.data() + 4, header.data(), sizeof(header));
		std::memcpy(data.data() + 20, json.data(), json.size());
		std::memcpy(data.data() + 20 + jsonSize, binHeader.data(), sizeof(binHeader));
		std::memcpy(data.data() + 28 + jsonSize, positions.data(), binSize);

		writePackerFile(path, data);
	}

	static bool isKTX2(const std::vector<uint8_t>& data) {
		return data.size() > 12 && data[0] == 0xAB && data[1] == 'K' && data[2] == 'T' && data[3] == 'X';
	}
} // namespace rawrbox

TEST_CASE("VPKPacker should behave as expected", "[rawrbox::VPKPacker]") {
	const std::string folder = "./rawrbox-tests-packer";
	std::filesystem::remove_all(folder);

	// INPUTS ---
	rawrbox::writeTGA(folder + "/base/textures/color.tga", 1);
	rawrbox::writeTGA(folder + "/base/textures/normal.tga", 2);
	rawrbox::writeTGA(folder + "/base/textures/atlas.tga", 3);
	rawrbox::writeTGA(folder + "/base/textures/height.vertex.tga", 4);
	rawrbox::writePackerFile(folder + "/base/Upper/Readme.TXT", "base readme");
	rawrbox::writePackerFile(folder + "/duplicate/upper/readme.txt", "duplicated readme");

	rawrbox::VPKPackSettings settings = {};
	settings.inputs = {{"assets", folder + "/base"}, {"assets", folder + "/duplicate"}};
	settings.output = folder + "/out/test_dir.vpk";
	settings.cache = folder + "/cache";
	settings.raw = {"assets/textures/atlas.tga"};
	settings.data = {"assets/textures/normal.tga"};
	settings.preload = {"assets/upper/**"};
	// --------

	SECTION("rawrbox::VPKPacker") {
		auto missing = settings;
		missing.inputs = {{"assets", folder + "/missing"}};
		REQUIRE_THROWS(rawrbox::VPKPacker(missing));

		auto noOutput = settings;
		noOutput.output.clear();
		REQUIRE_THROWS(rawrbox::VPKPacker(noOutput));
	}

	SECTION("rawrbox::VPKPacker::collectFiles") {
		rawrbox::TestPacker packer(settings);
		packer.collectFiles();

		REQUIRE(packer.getFiles().size() == 5);
		REQUIRE(packer.getFile("assets/textures/color.tga").type == rawrbox::VPKPackType::TEXTURE);
		REQUIRE_FALSE(packer.getFile("assets/textures/color.tga").data);
		REQUIRE(packer.getFile("assets/textures/normal.tga").data);
		REQUIRE(packer.getFile("assets/textures/atlas.tga").type == rawrbox::VPKPackType::COPY);
		REQUIRE(packer.getFile("assets/textures/height.vertex.tga").type == rawrbox::VPKPackType::COPY);

		// Duplicated path, first input wins ---
		REQUIRE(rawrbox::VFS::normalize(packer.getFile("assets/upper/readme.txt").source) == rawrbox::VFS::normalize(folder + "/base/Upper/Readme.TXT"));
	}

	SECTION("rawrbox::VPKPacker::collectFiles (glTF data textures)") {
		rawrbox::writeTGA(folder + "/models/albedo.tga", 5);
		rawrbox::writeTGA(folder + "/models/my normal.tga", 6);
		rawrbox::writePackerFile(folder + "/models/scene.gltf", R"({"asset":{"version":"2.0"},"images":[{"uri":"albedo.tga"},{"uri":"my%20normal.tga"}],"textures":[{"source":0},{"source":1}],)"
									R"("materials":[{"pbrMetallicRoughness":{"baseColorTexture":{"index":0}},"normalTexture":{"index":1}}]})");

		auto models = settings;
		models.inputs = {{"models", folder + "/models"}};

		rawrbox::TestPacker packer(models);
		packer.collectFiles();

		REQUIRE(packer.getFile("models/scene.gltf").type == rawrbox::VPKPackType::MODEL);
		REQUIRE_FALSE(packer.getFile("models/albedo.tga").data);
		REQUIRE(packer.getFile("models/my normal.tga").data);
	}

	SECTION("rawrbox::VPKPacker::getCacheKey") {
		rawrbox::writeTGA(folder + "/base/textures/copy.tga", 1); // Same bytes as color.tga

		rawrbox::TestPacker packer(settings);
		packer.collectFiles();

		const auto color = packer.getCacheKey(packer.getFile("assets/textures/color.tga"));
		REQUIRE(color == packer.getCacheKey(packer.getFile("assets/textures/color.tga")));
		REQUIRE(color != packer.getCacheKey(packer.getFile("assets/textures/copy.tga")));

		auto compressed = settings;
		compressed.compress = true;

		rawrbox::TestPacker compressedPacker(compressed);
		compressedPacker.collectFiles();
		REQUIRE(color != compressedPacker.getCacheKey(compressedPacker.getFile("assets/textures/color.tga")));
	}

	SECTION("rawrbox::VPKPacker::pack") {
		rawrbox::TestPacker packer(settings);
		packer.pack();

		{
			rawrbox::VPK vpk(settings.output);
			REQUIRE(vpk.getEntries().size() == 5);

			REQUIRE(rawrbox::isKTX2(vpk.read(*vpk.getEntry("assets/textures/color.tga"))));
			REQUIRE(rawrbox::isKTX2(vpk.read(*vpk.getEntry("assets/textures/normal.tga"))));
			REQUIRE(vpk.read(*vpk.getEntry("assets/textures/atlas.tga")) == rawrbox::readPackerFile(folder + "/base/textures/atlas.tga"));
			REQUIRE(vpk.read(*vpk.getEntry("assets/textures/height.vertex.tga")) == rawrbox::readPackerFile(folder + "/base/textures/height.vertex.tga"));

			const auto* readme = vpk.getEntry("assets/upper/readme.txt");
			REQUIRE(readme->archive == 0x7FFF);
			REQUIRE(vpk.read(*readme) == rawrbox::readPackerFile(folder + "/base/Upper/Readme.TXT"));
		}

		// Repack test ---
		const auto before = rawrbox::readPackerFile(settings.output.generic_string());

		rawrbox::TestPacker repacker(settings);
		repacker.pack();

		for (const auto& file : repacker.getFiles()) {
			if (file.type != rawrbox::VPKPackType::COPY) REQUIRE(file.cached);
		}

		REQUIRE(rawrbox::readPackerFile(settings.output.generic_string()) == before);
		// ------------
	}

#ifdef RAWRBOX_SCRIPTING
	SECTION("rawrbox::VPKPacker::pack (scripts)") {
		rawrbox::writePackerFile(folder + "/lua/init.lua", "RAWRBOX = 1");

		auto scripts = settings;
		scripts.inputs = {{"lua", folder + "/lua"}};

		rawrbox::TestPacker packer(scripts);
		packer.pack();

		rawrbox::VPK vpk(scripts.output);

		const auto script = vpk.read(*vpk.getEntry("lua/init.lua"));
		REQUIRE(std::string(script.begin(), script.end()).starts_with("Luau")); // Horrible, but o well

		rawrbox::writePackerFile(folder + "/lua/broken.lua", "bad lua");

		rawrbox::TestPacker broken(scripts);
		REQUIRE_THROWS(broken.pack());
	}
#endif

	SECTION("rawrbox::VPKPacker::pack (model)") {
		rawrbox::writeGLB(folder + "/models/triangle.glb");

		auto models = settings;
		models.inputs = {{"models", folder + "/models"}};

		rawrbox::TestPacker packer(models);
		packer.pack();

		rawrbox::VPK vpk(models.output);
		const auto glb = vpk.read(*vpk.getEntry("models/triangle.glb"));
		REQUIRE(glb.size() > 20);
		REQUIRE(std::memcmp(glb.data(), "glTF", 4) == 0);

		const std::string json(std::bit_cast<const char*>(glb.data()) + 20, glb.size() - 20);
		REQUIRE(json.find("meshopt_compression") != std::string::npos);
	}

	// CLEANUP ----
	std::filesystem::remove_all(folder);
	// ------------
}