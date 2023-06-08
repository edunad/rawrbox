#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Matrix4x4 should behave as expected", "[rawrbox::Matrix4x4]") {

	SECTION("rawrbox::Matrix4x4") {
		rawrbox::Matrix4x4 q = {};
		REQUIRE(q.size() == 16);

		// Identity by default
		for (size_t i = 0; i < q.size(); i++) {
			if (i == 0 || i == 5 || i == 10 || i == 15)
				REQUIRE(q[i] == 1.F);
			else
				REQUIRE(q[i] == 0.F);
		}
	}

	SECTION("rawrbox::Matrix4x4::zero") {
		rawrbox::Matrix4x4 q = {};
		q.zero();

		REQUIRE(q.size() == 16);
		for (size_t i = 0; i < q.size(); i++) {
			REQUIRE(q[i] == 0.F);
		}
	}

	SECTION("rawrbox::Matrix4x4::translate") {
		rawrbox::Matrix4x4 q = {};
		q.translate({0, 4, 10});

		REQUIRE(q[12] == 0);
		REQUIRE(q[13] == 4);
		REQUIRE(q[14] == 10);
	}

	SECTION("rawrbox::Matrix4x4::scale") {
		rawrbox::Matrix4x4 q = {};
		q.scale({0, 4, 10});

		REQUIRE(q[0] == 0);
		REQUIRE(q[5] == 4);
		REQUIRE(q[10] == 10);
	}

	SECTION("rawrbox::Matrix4x4::rotate") {
		rawrbox::Matrix4x4 q = {};
		q.rotate({0, 4, 10, 2});

		REQUIRE(q[0] == -231.0F);
		REQUIRE(q[1] == 40.0F);
		REQUIRE(q[2] == -16.0F);

		REQUIRE(q[3] == 0);

		REQUIRE(q[4] == -40.0F);
		REQUIRE(q[5] == -199.0F);
		REQUIRE(q[6] == 80.0F);

		REQUIRE(q[7] == 0);

		REQUIRE(q[8] == 16.0F);
		REQUIRE(q[9] == 80.0F);
		REQUIRE(q[10] == -31.0F);

		REQUIRE(q[11] == 0);
		REQUIRE(q[12] == 0);
		REQUIRE(q[13] == 0);
		REQUIRE(q[14] == 0);

		REQUIRE(q[15] == 1.0F);
	}

	SECTION("rawrbox::Matrix4x4::inverse") {
		rawrbox::Matrix4x4 q(std::array<float, 16>{2.F, 4.F, 1.F, 2.F, 4.F, 6.F, 7.F, 4.F, 3.F, 2.F, 5.F, 9.F, 1.F, 2.F, 3.F, 3.F});
		q.inverse();

		REQUIRE_THAT(q[0], Catch::Matchers::WithinAbs(-0.03571F, 0.0001F));
		REQUIRE_THAT(q[1], Catch::Matchers::WithinAbs(0.39286F, 0.0001F));
		REQUIRE_THAT(q[2], Catch::Matchers::WithinAbs(0.30357F, 0.0001F));
		REQUIRE_THAT(q[3], Catch::Matchers::WithinAbs(-1.41071F, 0.0001F));
		REQUIRE_THAT(q[4], Catch::Matchers::WithinAbs(0.28571F, 0.0001F));
		REQUIRE_THAT(q[5], Catch::Matchers::WithinAbs(-0.14286F, 0.0001F));
		REQUIRE_THAT(q[6], Catch::Matchers::WithinAbs(-0.17857F, 0.0001F));
		REQUIRE_THAT(q[7], Catch::Matchers::WithinAbs(0.53571F, 0.0001F));
		REQUIRE_THAT(q[8], Catch::Matchers::WithinAbs(-0.28571F, 0.0001F));
		REQUIRE_THAT(q[9], Catch::Matchers::WithinAbs(0.14286F, 0.0001F));
		REQUIRE_THAT(q[10], Catch::Matchers::WithinAbs(-0.07143F, 0.0001F));
		REQUIRE_THAT(q[11], Catch::Matchers::WithinAbs(0.21429F, 0.0001F));
		REQUIRE_THAT(q[12], Catch::Matchers::WithinAbs(0.10714F, 0.0001F));
		REQUIRE_THAT(q[13], Catch::Matchers::WithinAbs(-0.17857F, 0.0001F));
		REQUIRE_THAT(q[14], Catch::Matchers::WithinAbs(0.08929F, 0.0001F));
		REQUIRE_THAT(q[15], Catch::Matchers::WithinAbs(0.23214F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::rotateX") {
		rawrbox::Matrix4x4 q = {};
		q.rotateX(rawrbox::MathUtils::toRad(45));

		REQUIRE_THAT(q[5], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[6], Catch::Matchers::WithinAbs(-0.70711F, 0.0001F));
		REQUIRE_THAT(q[9], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[10], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::rotateY") {
		rawrbox::Matrix4x4 q = {};
		q.rotateY(rawrbox::MathUtils::toRad(45));

		REQUIRE_THAT(q[0], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[2], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[8], Catch::Matchers::WithinAbs(-0.70711F, 0.0001F));
		REQUIRE_THAT(q[10], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::rotateZ") {
		rawrbox::Matrix4x4 q = {};
		q.rotateZ(rawrbox::MathUtils::toRad(45));

		REQUIRE_THAT(q[0], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[1], Catch::Matchers::WithinAbs(-0.70711F, 0.0001F));
		REQUIRE_THAT(q[4], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
		REQUIRE_THAT(q[5], Catch::Matchers::WithinAbs(0.70711F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::mul") {
		rawrbox::Matrix4x4 q = {};
		rawrbox::Matrix4x4 other(std::array<float, 16>{2.F, 4.F, 1.F, 2.F, 4.F, 6.F, 7.F, 4.F, 3.F, 2.F, 5.F, 9.F, 1.F, 2.F, 3.F, 3.F});
		rawrbox::Matrix4x4 q2 = q * other;
		q.mul(other);

		REQUIRE_THAT(q[0], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q[4], Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(q[8], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q[12], Catch::Matchers::WithinAbs(1.0F, 0.0001F));

		REQUIRE_THAT(q2[0], Catch::Matchers::WithinAbs(q[0], 0.0001F));
		REQUIRE_THAT(q2[4], Catch::Matchers::WithinAbs(q[4], 0.0001F));
		REQUIRE_THAT(q2[8], Catch::Matchers::WithinAbs(q[8], 0.0001F));
		REQUIRE_THAT(q2[12], Catch::Matchers::WithinAbs(q[12], 0.0001F));
	}
}
