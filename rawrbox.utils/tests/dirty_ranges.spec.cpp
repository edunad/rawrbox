#include <rawrbox/utils/dirty_ranges.hpp>

#include <catch2/catch_test_macros.hpp>

#include <tuple>

TEST_CASE("DirtyRanges should behave", "[rawrbox::DirtyRanges]") {
	SECTION("empty by default") {
		rawrbox::DirtyRanges ranges;

		REQUIRE(ranges.empty());
		REQUIRE_FALSE(ranges.full());
	}

	SECTION("merges near and overlapping ranges") {
		rawrbox::DirtyRanges ranges;

		ranges.push(100, 50);
		ranges.push(120, 100);
		ranges.push(300, 10);
		ranges.push(100000, 10);

		const auto& out = ranges.merge(1u << 20u);

		REQUIRE(out.size() == 2);
		REQUIRE(out[0].offset == 100);
		REQUIRE(out[0].end() == 310);
		REQUIRE(out[1].offset == 100000);
	}

	SECTION("clamps to the buffer size") {
		rawrbox::DirtyRanges ranges;

		ranges.push(50, 100);
		ranges.push(500, 10);

		const auto& out = ranges.merge(100);

		REQUIRE(ranges.full());
		REQUIRE(out.empty());
	}

	SECTION("degrades to full on coverage") {
		rawrbox::DirtyRanges ranges;
		ranges.push(0, 600);

		std::ignore = ranges.merge(1000);
		REQUIRE(ranges.full());
	}

	SECTION("degrades to full on fragmentation") {
		rawrbox::DirtyRanges ranges;
		for (uint64_t i = 0; i < 20; i++) {
			ranges.push(i * 100000, 8);
		}

		std::ignore = ranges.merge(10u << 20u);
		REQUIRE(ranges.full());
	}

	SECTION("clear resets full state") {
		rawrbox::DirtyRanges ranges;
		ranges.markFull();
		REQUIRE(ranges.full());

		ranges.clear();
		REQUIRE(ranges.empty());
		REQUIRE_FALSE(ranges.full());
	}
}
