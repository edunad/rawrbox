#include <rawrbox/network/network_var.hpp>

#include <catch2/catch_test_macros.hpp>

TEST_CASE("NetVar should behave as expected", "[rawrbox::NetVar]") {
	SECTION("rawrbox::NetVar<T>") {
		rawrbox::NetVar<float> net = 20.F;

		REQUIRE(net.isInitialized());
		REQUIRE(net.get() == 20.F);

		net = 55.F;
		REQUIRE(net.get() == 55.F);
	}

	SECTION("rawrbox::NetVar<T>::isDirty") {
		rawrbox::NetVar<float> net = 20.F;

		REQUIRE(net.get() == 20.F);
		REQUIRE(!net.isDirty(0));

		auto crc = net.getCRC();

		net.set(55.F, false);
		REQUIRE(net.isDirty(crc));
	}
}
