
#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/math/utils/math.hpp>

#include <stdexcept>

namespace rawrbox {
	BezierCurve::BezierCurve(const std::array<rawrbox::Vector3f, 4>& points, float subDivisions) : _subDivisions(subDivisions), _points(points) {
		if (subDivisions <= 0) throw std::runtime_error("Subdivisions must be a positive number.");
		this->generateSamples();
	}

	// PRIVATE -----
	void BezierCurve::generateSamples() {
		this->_sampleLenghts.clear();
		this->_sampleLenghts.push_back(0);

		rawrbox::Vector3f prevPoint = this->_points[0];
		rawrbox::Vector3f pt = {};
		float total = 0;

		float step = 1.0F / this->_subDivisions;
		// NOLINTBEGIN(clang-analyzer-security.FloatLoopCounter)
		for (float f = step; f < 1.0F; f += step) {
			pt = this->getPoint(f);
			total += (pt - prevPoint).length();

			this->_sampleLenghts.push_back(total);
			prevPoint = pt;
		}
		// NOLINTEND(clang-analyzer-security.FloatLoopCounter)

		pt = this->getPoint(1);
		this->_sampleLenghts.push_back(total + (pt - prevPoint).length());
	}

	// -----
	[[nodiscard]] const std::array<rawrbox::Vector3f, 4>& BezierCurve::getPoints() const { return this->_points; }

	rawrbox::Vector3f BezierCurve::getNormal(float t, Vector3 up) {
		return rawrbox::BezierCurve::calculateNormal(this->getTangent(t), up);
	}

	rawrbox::Vector3f BezierCurve::getTangent(float t) {
		float t2 = t * t;
		float it = (1.F - t);
		float it2 = it * it;

		return this->calculateTangent(t, t2, it2);
	}

	rawrbox::Vector3f BezierCurve::calculateNormal(rawrbox::Vector3f tangent, rawrbox::Vector3f up) {
		rawrbox::Vector3f binormal = up.cross(tangent);
		return tangent.cross(binormal);
	}

	rawrbox::Vector3f BezierCurve::calculateTangent(float t, float t2, float it2) {
		return (this->_points[0] * -it2 +
			this->_points[1] * (t * (3 * t - 4) + 1) +
			this->_points[2] * (-3 * t2 + t * 2) +
			this->_points[3] * t2)
		    .normalized();
	}

	rawrbox::Vector3f BezierCurve::calculatePoint(float t, float t2, float t3, float it, float it2, float it3) {
		return this->_points[0] * (it3) +
		       this->_points[1] * (3 * it2 * t) +
		       this->_points[2] * (3 * it * t2) +
		       this->_points[3] * t3;
	}

	rawrbox::Vector3f BezierCurve::getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal, rawrbox::Vector4f& orientation) {
		float t2 = t * t;
		float t3 = t2 * t;
		float it = (1.F - t);
		float it2 = it * it;
		float it3 = it * it * it;

		tangent = this->calculateTangent(t, t2, it2);
		normal = this->calculateNormal(tangent, rawrbox::Vector3f::up());
		orientation = rawrbox::Vector4f::lookRotation(tangent, normal);

		return this->calculatePoint(t, t2, t3, it, it2, it3);
	}

	rawrbox::Vector3f BezierCurve::getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal) {
		float t2 = t * t;
		float t3 = t2 * t;
		float it = (1 - t);
		float it2 = it * it;
		float it3 = it * it * it;

		tangent = this->calculateTangent(t, t2, it2);
		normal = this->calculateNormal(tangent, rawrbox::Vector3f::up());

		return this->calculatePoint(t, t2, t3, it, it2, it3);
	}

	rawrbox::Vector3f BezierCurve::getPoint(float t, rawrbox::Vector3f& tangent) {
		float t2 = t * t;
		float t3 = t2 * t;
		float it = (1 - t);
		float it2 = it * it;
		float it3 = it * it * it;

		tangent = this->calculateTangent(t, t2, it2);
		return this->calculatePoint(t, t2, t3, it, it2, it3);
	}

	rawrbox::Vector3f BezierCurve::getPoint(float t) {
		float t2 = t * t;
		float t3 = t2 * t;
		float it = (1 - t);
		float it2 = it * it;
		float it3 = it * it * it;

		return this->calculatePoint(t, t2, t3, it, it2, it3);
	}

	rawrbox::OrientedPoint BezierCurve::getOrientedPoint(float t) {
		rawrbox::Vector3f tangent = {};
		rawrbox::Vector3f normal = {};
		rawrbox::Vector4f orientation = {};

		Vector3 point = this->getPoint(t, tangent, normal, orientation);
		return {point, orientation, rawrbox::MathUtils::sample(this->_sampleLenghts, t)};
	}

	std::vector<rawrbox::OrientedPoint> BezierCurve::generatePath() {
		std::vector<rawrbox::OrientedPoint> paths = {};

		float step = 1.0F / this->_subDivisions;
		// NOLINTBEGIN(clang-analyzer-security.FloatLoopCounter)
		for (float f = 0; f < 1; f += step) {
			paths.push_back(this->getOrientedPoint(f));
		}
		// NOLINTEND(clang-analyzer-security.FloatLoopCounter)

		paths.push_back(this->getOrientedPoint(1));
		return paths;
	}
} // namespace rawrbox
