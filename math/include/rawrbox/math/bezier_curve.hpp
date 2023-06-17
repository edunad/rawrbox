#pragma once
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <stdexcept>
#include <vector>

namespace rawrbox {

	struct OrientedPoint {
		rawrbox::Vector3f position = {};
		rawrbox::Vector4f rotation = {};
		float vCoordinate;

		OrientedPoint(rawrbox::Vector3f position, rawrbox::Vector4f rotation, float vCoordinate = 0) : position(position), rotation(rotation), vCoordinate(vCoordinate) {}

		rawrbox::Vector3f LocalToWorld(const rawrbox::Vector3f& point) {
			return position + rotation * point;
		}

		rawrbox::Vector3f WorldToLocal(const rawrbox::Vector3f& point) {
			return rotation.inverse() * (point - position);
		}

		rawrbox::Vector3f LocalToWorldDirection(const rawrbox::Vector3f& dir) {
			return rotation * dir;
		}
	};

	class BezierCurve {
	protected:
		std::vector<float> _sampleLenghts = {};
		std::vector<rawrbox::Vector3f> _points = {};

		void generateSamples() {
			this->_sampleLenghts.clear();
			this->_sampleLenghts.reserve(10);
			this->_sampleLenghts.push_back(0);

			rawrbox::Vector3f prevPoint = this->_points[0];
			rawrbox::Vector3f pt = {};
			float total = 0;

			float step = 1.0F / 10.F;
			for (float f = step; f < 1.0F; f += step) {
				pt = this->getPoint(f);
				total += (pt - prevPoint).length();

				this->_sampleLenghts.push_back(total);
			}

			pt = this->getPoint(1);
			this->_sampleLenghts.push_back(total + (pt - prevPoint).length());
		}

	public:
		BezierCurve() = default;
		explicit BezierCurve(const std::vector<rawrbox::Vector3f>& points) : _points(points) {
			if (this->_points.size() == 4) this->generateSamples();
		}

		void setPoints(const std::vector<rawrbox::Vector3f>& points) {
			this->_points = points;
			if (this->_points.size() == 4) this->generateSamples();
		}

		[[nodiscard]] const std::vector<rawrbox::Vector3f>& getPoints() const { return this->_points; }

		rawrbox::Vector3f getNormal(float t, Vector3 up) {
			return this->calculateNormal(this->getTangent(t), up);
		}

		rawrbox::Vector3f getTangent(float t) {
			float t2 = t * t;
			float it = (1.F - t);
			float it2 = it * it;

			return this->calculateTangent(t, t2, it2);
		}

		rawrbox::Vector3f calculateNormal(rawrbox::Vector3f tangent, rawrbox::Vector3f up) {
			rawrbox::Vector3f binormal = up.cross(tangent);
			return tangent.cross(binormal);
		}

		rawrbox::Vector3f calculateTangent(float t, float t2, float it2) {
			return (this->_points[0] * -it2 +
				this->_points[1] * (t * (3 * t - 4) + 1) +
				this->_points[2] * (-3 * t2 + t * 2) +
				this->_points[3] * t2)
			    .normalized();
		}

		rawrbox::Vector3f calculatePoint(float t, float t2, float t3, float it, float it2, float it3) {
			return this->_points[0] * (it3) +
			       this->_points[1] * (3 * it2 * t) +
			       this->_points[2] * (3 * it * t2) +
			       this->_points[3] * t3;
		}

		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal, rawrbox::Vector4f& orientation) {
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

		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal) {
			float t2 = t * t;
			float t3 = t2 * t;
			float it = (1 - t);
			float it2 = it * it;
			float it3 = it * it * it;

			tangent = this->calculateTangent(t, t2, it2);
			normal = this->calculateNormal(tangent, rawrbox::Vector3f::up());

			return this->calculatePoint(t, t2, t3, it, it2, it3);
		}

		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent) {
			float t2 = t * t;
			float t3 = t2 * t;
			float it = (1 - t);
			float it2 = it * it;
			float it3 = it * it * it;

			tangent = this->calculateTangent(t, t2, it2);
			return this->calculatePoint(t, t2, t3, it, it2, it3);
		}

		rawrbox::Vector3f getPoint(float t) {
			float t2 = t * t;
			float t3 = t2 * t;
			float it = (1 - t);
			float it2 = it * it;
			float it3 = it * it * it;

			return this->calculatePoint(t, t2, t3, it, it2, it3);
		}

		rawrbox::OrientedPoint getOrientedPoint(float t) {
			rawrbox::Vector3f tangent = {};
			rawrbox::Vector3f normal = {};
			rawrbox::Vector4f orientation = {};

			Vector3 point = this->getPoint(t, tangent, normal, orientation);
			return {point, orientation, rawrbox::MathUtils::sample(this->_sampleLenghts, t)};
		}

		std::vector<rawrbox::OrientedPoint> generatePath(float subDivisions) {
			std::vector<rawrbox::OrientedPoint> paths = {};

			float step = 1.0F / subDivisions;
			for (float f = 0; f < 1; f += step) {
				paths.push_back(this->getOrientedPoint(f));
			}

			paths.push_back(this->getOrientedPoint(1));
			return paths;
		}
	};
} // namespace rawrbox
