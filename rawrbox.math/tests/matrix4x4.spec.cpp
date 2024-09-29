#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

TEST_CASE("Matrix4x4 should behave as expected", "[rawrbox::Matrix4x4]") {

	SECTION("rawrbox::Matrix4x4") {
		rawrbox::Matrix4x4 q = {};

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

		for (size_t i = 0; i < q.size(); i++) {
			REQUIRE(q[i] == 0.F);
		}
	}

	SECTION("rawrbox::Matrix4x4::size") {
		rawrbox::Matrix4x4 q = {};
		REQUIRE(q.size() == 16);
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

	SECTION("rawrbox::Matrix4x4::getScale") {
		rawrbox::Matrix4x4 q = {};
		q.scale({0, 4, 10});

		REQUIRE(q.getScale() == rawrbox::Vector3f(0, 4, 10));
	}

	SECTION("rawrbox::Matrix4x4::getPos") {
		rawrbox::Matrix4x4 q = {};
		q.translate({-4, 12, 5});

		REQUIRE(q.getPos() == rawrbox::Vector3f(-4, 12, 5));
	}

	SECTION("rawrbox::Matrix4x4::getRotation") {
		rawrbox::Matrix4x4 q = {};
		q.rotate({0, 4, 10, 2});

		rawrbox::Vector4f rotation = q.getRotation();

		REQUIRE_THAT(rotation.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(rotation.y, Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(rotation.z, Catch::Matchers::WithinAbs(10.0F, 0.0001F));
		REQUIRE_THAT(rotation.w, Catch::Matchers::WithinAbs(2.0F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::getForward") {
		// Test case 1: Identity matrix
		rawrbox::Matrix4x4 identityMatrix;

		rawrbox::Vector3f forward = identityMatrix.getForward();
		REQUIRE_THAT(forward.x, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(forward.y, Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(forward.z, Catch::Matchers::WithinAbs(1.0F, 0.0001F));

		// Test case 2: Custom matrix
		rawrbox::Matrix4x4 customMatrix;
		customMatrix.mtx = {
		    1.0f,
		    0.0f,
		    0.0f,
		    0.0f,
		    0.0f,
		    1.0f,
		    0.0f,
		    0.0f,
		    0.5f,
		    0.5f,
		    0.5f,
		    0.0f,
		    0.0f,
		    0.0f,
		    0.0f,
		    1.0f};

		forward = customMatrix.getForward();
		REQUIRE_THAT(forward.x, Catch::Matchers::WithinAbs(0.5F, 0.0001F));
		REQUIRE_THAT(forward.y, Catch::Matchers::WithinAbs(0.5F, 0.0001F));
		REQUIRE_THAT(forward.z, Catch::Matchers::WithinAbs(0.5F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::transpose") {
		rawrbox::Matrix4x4 q(std::array<float, 16>{2.F, 4.F, 1.F, 2.F, 4.F, 6.F, 7.F, 4.F, 3.F, 2.F, 5.F, 9.F, 1.F, 2.F, 3.F, 3.F});
		q.transpose();

		REQUIRE(q[0] == 2.F);
		REQUIRE(q[1] == 4.0F);
		REQUIRE(q[2] == 3.0F);
		REQUIRE(q[3] == 1.0F);

		REQUIRE(q[4] == 4.0F);
		REQUIRE(q[5] == 6.0F);
		REQUIRE(q[6] == 2.0F);
		REQUIRE(q[7] == 2.0F);

		REQUIRE(q[8] == 1.0F);
		REQUIRE(q[9] == 7.0F);
		REQUIRE(q[10] == 5.0F);
		REQUIRE(q[11] == 3.0F);

		REQUIRE(q[12] == 2.0F);
		REQUIRE(q[13] == 4.0F);
		REQUIRE(q[14] == 9.0F);
		REQUIRE(q[15] == 3.0F);
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

	SECTION("rawrbox::Matrix4x4::*") {
		rawrbox::Matrix4x4 q = {};
		rawrbox::Matrix4x4 other(std::array<float, 16>{2.F, 4.F, 1.F, 2.F, 4.F, 6.F, 7.F, 4.F, 3.F, 2.F, 5.F, 9.F, 1.F, 2.F, 3.F, 3.F});

		rawrbox::Matrix4x4 q2 = q * other;
		q *= other;

		REQUIRE_THAT(q[0], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q[4], Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(q[8], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q[12], Catch::Matchers::WithinAbs(1.0F, 0.0001F));

		REQUIRE_THAT(q2[0], Catch::Matchers::WithinAbs(q[0], 0.0001F));
		REQUIRE_THAT(q2[4], Catch::Matchers::WithinAbs(q[4], 0.0001F));
		REQUIRE_THAT(q2[8], Catch::Matchers::WithinAbs(q[8], 0.0001F));
		REQUIRE_THAT(q2[12], Catch::Matchers::WithinAbs(q[12], 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::add") {
		rawrbox::Matrix4x4 q = {};
		rawrbox::Matrix4x4 other(std::array<float, 16>{2.F, 4.F, 1.F, 2.F, 4.F, 6.F, 7.F, 4.F, 3.F, 2.F, 5.F, 9.F, 1.F, 2.F, 3.F, 3.F});

		rawrbox::Matrix4x4 q2 = q + other;
		rawrbox::Matrix4x4 q3 = q + rawrbox::Vector3f(10, 2, 3);

		REQUIRE_THAT(q2[0], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q2[1], Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(q2[2], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(q2[3], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q2[4], Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(q2[5], Catch::Matchers::WithinAbs(7.0F, 0.0001F));
		REQUIRE_THAT(q2[6], Catch::Matchers::WithinAbs(7.0F, 0.0001F));
		REQUIRE_THAT(q2[7], Catch::Matchers::WithinAbs(4.0F, 0.0001F));
		REQUIRE_THAT(q2[8], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q2[9], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q2[10], Catch::Matchers::WithinAbs(6.0F, 0.0001F));
		REQUIRE_THAT(q2[11], Catch::Matchers::WithinAbs(9.0F, 0.0001F));
		REQUIRE_THAT(q2[12], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(q2[13], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q2[14], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q2[15], Catch::Matchers::WithinAbs(4.0F, 0.0001F));

		REQUIRE_THAT(q3[0], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(q3[1], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[2], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[5], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(q3[6], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[8], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[9], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[10], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(q3[11], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(q3[12], Catch::Matchers::WithinAbs(10.0F, 0.0001F));
		REQUIRE_THAT(q3[13], Catch::Matchers::WithinAbs(2.0F, 0.0001F));
		REQUIRE_THAT(q3[14], Catch::Matchers::WithinAbs(3.0F, 0.0001F));
		REQUIRE_THAT(q3[15], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::lookAt") {
		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = false;
		auto viewLH = rawrbox::Matrix4x4::mtxLookAt({0, 0, 0}, {10, 10, 10}, {0, 1, 0});

		REQUIRE_THAT(viewLH[0], Catch::Matchers::WithinAbs(0.707106769F, 0.0001F));
		REQUIRE_THAT(viewLH[1], Catch::Matchers::WithinAbs(-0.408248246F, 0.0001F));
		REQUIRE_THAT(viewLH[2], Catch::Matchers::WithinAbs(0.577350199F, 0.0001F));
		REQUIRE_THAT(viewLH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[5], Catch::Matchers::WithinAbs(0.816496491F, 0.0001F));
		REQUIRE_THAT(viewLH[6], Catch::Matchers::WithinAbs(0.577350199F, 0.0001F));
		REQUIRE_THAT(viewLH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[8], Catch::Matchers::WithinAbs(-0.707106769F, 0.0001F));
		REQUIRE_THAT(viewLH[9], Catch::Matchers::WithinAbs(-0.408248246F, 0.0001F));
		REQUIRE_THAT(viewLH[10], Catch::Matchers::WithinAbs(0.577350199F, 0.0001F));
		REQUIRE_THAT(viewLH[11], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[12], Catch::Matchers::WithinAbs(-0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[13], Catch::Matchers::WithinAbs(-0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[14], Catch::Matchers::WithinAbs(-0.0F, 0.0001F));
		REQUIRE_THAT(viewLH[15], Catch::Matchers::WithinAbs(1.0F, 0.0001F));

		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = true;
		auto viewRH = rawrbox::Matrix4x4::mtxLookAt({0, 0, 0}, {10, 10, 10}, {0, 1, 0});

		REQUIRE_THAT(viewRH[0], Catch::Matchers::WithinAbs(-0.707106769F, 0.0001F));
		REQUIRE_THAT(viewRH[1], Catch::Matchers::WithinAbs(-0.408248246F, 0.0001F));
		REQUIRE_THAT(viewRH[2], Catch::Matchers::WithinAbs(-0.577350199F, 0.0001F));
		REQUIRE_THAT(viewRH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[5], Catch::Matchers::WithinAbs(0.816496491F, 0.0001F));
		REQUIRE_THAT(viewRH[6], Catch::Matchers::WithinAbs(-0.577350199F, 0.0001F));
		REQUIRE_THAT(viewRH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[8], Catch::Matchers::WithinAbs(0.707106769F, 0.0001F));
		REQUIRE_THAT(viewRH[9], Catch::Matchers::WithinAbs(-0.408248246F, 0.0001F));
		REQUIRE_THAT(viewRH[10], Catch::Matchers::WithinAbs(-0.577350199F, 0.0001F));
		REQUIRE_THAT(viewRH[11], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[12], Catch::Matchers::WithinAbs(-0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[13], Catch::Matchers::WithinAbs(-0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[14], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(viewRH[15], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::mtxProj") {
		float W = 1024;
		float H = 768;
		float aspect = W / H;

		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = false;
		auto projLH = rawrbox::Matrix4x4::mtxProj(60.F, aspect, 0.01F, 100.F);

		REQUIRE_THAT(projLH[0], Catch::Matchers::WithinAbs(1.29903805F, 0.0001F));
		REQUIRE_THAT(projLH[1], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[2], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[5], Catch::Matchers::WithinAbs(1.73205090F, 0.0001F));
		REQUIRE_THAT(projLH[6], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[8], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[9], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[10], Catch::Matchers::WithinAbs(1.00020003F, 0.0001F));
		REQUIRE_THAT(projLH[11], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
		REQUIRE_THAT(projLH[12], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[13], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projLH[14], Catch::Matchers::WithinAbs(-0.0200020000F, 0.0001F));
		REQUIRE_THAT(projLH[15], Catch::Matchers::WithinAbs(0.0F, 0.0001F));

		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = true;
		auto projRH = rawrbox::Matrix4x4::mtxProj(60.F, aspect, 0.01F, 100.F);

		REQUIRE_THAT(projRH[0], Catch::Matchers::WithinAbs(1.29903805F, 0.0001F));
		REQUIRE_THAT(projRH[1], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[2], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[5], Catch::Matchers::WithinAbs(1.73205090F, 0.0001F));
		REQUIRE_THAT(projRH[6], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[8], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[9], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[10], Catch::Matchers::WithinAbs(-1.00020003F, 0.0001F));
		REQUIRE_THAT(projRH[11], Catch::Matchers::WithinAbs(-1.0F, 0.0001F));
		REQUIRE_THAT(projRH[12], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[13], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(projRH[14], Catch::Matchers::WithinAbs(-0.0200020000F, 0.0001F));
		REQUIRE_THAT(projRH[15], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
	}

	SECTION("rawrbox::Matrix4x4::mtxOrtho") {
		float W = 1024;
		float H = 768;

		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = false;
		auto orthoLH = rawrbox::Matrix4x4::mtxOrtho(0, 0, W, H, 0.01F, 100.F);

		REQUIRE_THAT(orthoLH[1], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[2], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[5], Catch::Matchers::WithinAbs(-0.00781250000F, 0.0001F));
		REQUIRE_THAT(orthoLH[6], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[8], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[9], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[10], Catch::Matchers::WithinAbs(0.0200020000F, 0.0001F));
		REQUIRE_THAT(orthoLH[11], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[13], Catch::Matchers::WithinAbs(7.0F, 0.0001F));
		REQUIRE_THAT(orthoLH[14], Catch::Matchers::WithinAbs(-1.00020003F, 0.0001F));
		REQUIRE_THAT(orthoLH[15], Catch::Matchers::WithinAbs(1.0F, 0.0001F));

		rawrbox::Matrix4x4::MTX_RIGHT_HANDED = true;
		auto orthoRH = rawrbox::Matrix4x4::mtxOrtho(0, 0, W, H, 0.01F, 100.F);

		REQUIRE_THAT(orthoRH[1], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[2], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[3], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[4], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[5], Catch::Matchers::WithinAbs(-0.00781250000F, 0.0001F));
		REQUIRE_THAT(orthoRH[6], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[7], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[8], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[9], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[10], Catch::Matchers::WithinAbs(-0.0200020000F, 0.0001F));
		REQUIRE_THAT(orthoRH[11], Catch::Matchers::WithinAbs(0.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[13], Catch::Matchers::WithinAbs(7.0F, 0.0001F));
		REQUIRE_THAT(orthoRH[14], Catch::Matchers::WithinAbs(-1.00020003F, 0.0001F));
		REQUIRE_THAT(orthoRH[15], Catch::Matchers::WithinAbs(1.0F, 0.0001F));
	}
}
