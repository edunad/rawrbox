#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/streaming.hpp>
#include <rawrbox/render/textures/utils/stbi.hpp>

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <span>
#include <vector>

class TestStreaming : public rawrbox::TextureStreaming {
public:
	using rawrbox::TextureStreaming::TextureStreaming;

	[[nodiscard]] const std::vector<rawrbox::AABBu>& dirty() const { return this->_dirty; }
	[[nodiscard]] bool dirtyAll() const { return this->_dirtyAll; }
	void flushed() {
		this->_dirty.clear();
		this->_dirtyAll = false;
	}
};

TEST_CASE("STBI should behave as expected", "[rawrbox::STBI]") {
	SECTION("rawrbox::STBI::decode failures") {
		REQUIRE_THROWS(rawrbox::STBI::decode(std::filesystem::path("./missing-file.png")));
		REQUIRE_THROWS(rawrbox::STBI::decode(std::vector<uint8_t>{}));
		REQUIRE(rawrbox::STBI::decode(std::vector<uint8_t>{1, 2, 3, 4}).frames.empty());
		REQUIRE_THROWS(rawrbox::STBI::decode(nullptr, 0));
	}

	SECTION("rawrbox::STBI::encodePNG / info / decode") {
		std::vector<uint8_t> pixels = {};
		for (uint8_t i = 0; i < 3 * 2 * 4; i++) {
			pixels.push_back(static_cast<uint8_t>(i * 10));
		}

		const auto png = rawrbox::STBI::encodePNG({3, 2}, 4, pixels);
		REQUIRE_FALSE(png.empty());

		const auto info = rawrbox::STBI::info(png);
		REQUIRE(info.has_value());
		REQUIRE(info->size == rawrbox::Vector2u{3, 2});
		REQUIRE(info->channels == 4);

		const auto image = rawrbox::STBI::decode(png);
		REQUIRE(image.size == rawrbox::Vector2u{3, 2});
		REQUIRE(image.channels == 4);
		REQUIRE(image.frames.front().pixels == pixels);

		REQUIRE(rawrbox::STBI::encodePNG({3, 2}, 4, std::vector<uint8_t>(4)).empty());
		REQUIRE_FALSE(rawrbox::STBI::info(std::vector<uint8_t>{1, 2, 3}).has_value());
	}

	SECTION("rawrbox::STBI::encodePNG with a stride") {
		const std::vector<uint8_t> rows = {10, 20, 30, 40, 50, 60, 0, 0, 70, 80, 90, 100, 110, 120};

		const auto png = rawrbox::STBI::encodePNG({2, 2}, 3, rows, 8);
		REQUIRE_FALSE(png.empty());

		const auto image = rawrbox::STBI::decode(png);
		REQUIRE(image.size == rawrbox::Vector2u{2, 2});
		REQUIRE(image.frames.front().pixels == std::vector<uint8_t>{10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120});

		REQUIRE(rawrbox::STBI::encodePNG({2, 2}, 3, rows, 4).empty());
		REQUIRE(rawrbox::STBI::encodePNG({2, 2}, 3, std::span<const uint8_t>(rows).first(13), 8).empty());
	}

	SECTION("rawrbox::STBI::writePNG") {
		const std::filesystem::path path = "./rawrbox-tests-stbi/sub/out.png";
		std::filesystem::remove_all("./rawrbox-tests-stbi");

		const std::vector<uint8_t> pixels = {255, 0, 0, 0, 255, 0};
		REQUIRE(rawrbox::STBI::writePNG(path, {2, 1}, 3, pixels));

		const auto info = rawrbox::STBI::info(path);
		REQUIRE(info.has_value());
		REQUIRE(info->size == rawrbox::Vector2u{2, 1});
		REQUIRE(info->channels == 3);

		REQUIRE_FALSE(rawrbox::STBI::info("./rawrbox-tests-stbi/missing.png").has_value());
		std::filesystem::remove_all("./rawrbox-tests-stbi");
	}
}

TEST_CASE("TextureStreaming should behave as expected", "[rawrbox::TextureStreaming]") {
	TestStreaming texture({64, 64});
	texture.flushed();

	SECTION("rawrbox::TextureStreaming::markDirty merges what touches") {
		texture.markDirty(rawrbox::AABBu{0, 0, 4, 4});
		texture.markDirty(rawrbox::AABBu{4, 0, 4, 4});
		texture.markDirty(rawrbox::AABBu{32, 32, 2, 2});
		REQUIRE(texture.hasData());
		REQUIRE(texture.requiresUpdate());

		REQUIRE(texture.dirty().size() == 2);
		REQUIRE(texture.dirty()[0] == rawrbox::AABBu{0, 0, 8, 4});
		REQUIRE(texture.dirty()[1] == rawrbox::AABBu{32, 32, 2, 2});

		texture.markDirty(rawrbox::AABBu{2, 2, 32, 32});
		REQUIRE(texture.dirty().size() == 1);
		REQUIRE(texture.dirty()[0] == rawrbox::AABBu{0, 0, 34, 34});
	}

	SECTION("rawrbox::TextureStreaming::markDirty clamps and collapses") {
		texture.markDirty(rawrbox::AABBu{60, 60, 10, 10});
		REQUIRE(texture.dirty().front() == rawrbox::AABBu{60, 60, 4, 4});

		texture.markDirty(rawrbox::AABBu{80, 80, 4, 4});
		REQUIRE(texture.dirty().size() == 1);

		for (uint32_t i = 0; i < 10; i++) {
			texture.markDirty(rawrbox::AABBu{i * 6, 0, 1, 1});
		}

		REQUIRE(texture.dirty().size() <= 8);

		texture.markDirty(rawrbox::AABBu{0, 0, 64, 64});
		REQUIRE(texture.dirtyAll());
		REQUIRE(texture.dirty().empty());

		texture.flushed();
		texture.markDirty();
		REQUIRE(texture.dirtyAll());
	}

	SECTION("rawrbox::TextureStreaming::pixels") {
		REQUIRE(texture.pixels().size() == 64U * 64U * 4U);
		texture.clear();
		REQUIRE_FALSE(texture.hasData());
		REQUIRE_FALSE(texture.requiresUpdate());
	}
}
