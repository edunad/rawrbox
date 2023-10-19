#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/utils/pack.hpp>

namespace rawrbox {
	struct Mesh2DShape {
		std::vector<rawrbox::Vector2f> vertex = {};
		std::vector<rawrbox::Vector2f> normal = {};
		std::vector<float> u = {};

		Mesh2DShape() = default;

		std::vector<int> getLineSegments() {
			std::vector<int> segments = {};

			for (size_t i = 0; i < this->vertex.size() - 1; i++) {
				segments.push_back(static_cast<int>(i));
				segments.push_back(static_cast<int>(i) + 1);
			}

			return segments;
		}
	};

	class Spline : public rawrbox::ModelBase {
	protected:
		std::vector<std::unique_ptr<rawrbox::BezierCurve>> _curves = {};
		std::unique_ptr<rawrbox::Mesh2DShape> _shape = nullptr;
		float _subDivisions = 8.F;

		virtual void addBezier(std::array<rawrbox::Vector3f, 4> points);

	public:
		explicit Spline(float subDivisions = 8.F);
		Spline(const Spline&) = delete;
		Spline(Spline&&) = delete;
		Spline& operator=(const Spline&) = delete;
		Spline& operator=(Spline&&) = delete;
		~Spline() override = default;

		virtual void setExtrudeVerts(rawrbox::Mesh2DShape shape);
		virtual void setTexture(rawrbox::TextureBase* texture);
		virtual void setColor(const rawrbox::Color& col);

		virtual rawrbox::Mesh& getMesh();

		virtual const std::vector<std::unique_ptr<rawrbox::BezierCurve>>& getCurves();

		virtual void addPoint(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4);
		virtual void addPoint(const rawrbox::Vector4f& start, const rawrbox::Vector4f& end, float distance = 0.5F);

		virtual void generateMesh();

		void draw() override;
	};
} // namespace rawrbox
