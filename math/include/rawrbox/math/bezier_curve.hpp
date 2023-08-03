#pragma once
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>

#include <array>
#include <vector>

// Based off https://pastebin.com/raw/vJhXxfH9
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
		float _subDivisions = 8.F;

		std::vector<float> _sampleLenghts = {};
		std::array<rawrbox::Vector3f, 4> _points = {};

		void generateSamples();

	public:
		BezierCurve() = default;
		explicit BezierCurve(const std::array<rawrbox::Vector3f, 4>& points, float subDivisions = 8.F);

		[[nodiscard]] const std::array<rawrbox::Vector3f, 4>& getPoints() const;

		rawrbox::Vector3f getNormal(float t, Vector3 up);
		rawrbox::Vector3f getTangent(float t);

		rawrbox::Vector3f calculateNormal(rawrbox::Vector3f tangent, rawrbox::Vector3f up);
		rawrbox::Vector3f calculateTangent(float t, float t2, float it2);
		rawrbox::Vector3f calculatePoint(float t, float t2, float t3, float it, float it2, float it3);

		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal, rawrbox::Vector4f& orientation);
		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent, rawrbox::Vector3f& normal);
		rawrbox::Vector3f getPoint(float t, rawrbox::Vector3f& tangent);
		rawrbox::Vector3f getPoint(float t);

		rawrbox::OrientedPoint getOrientedPoint(float t);

		std::vector<rawrbox::OrientedPoint> generatePath();
	};
} // namespace rawrbox
