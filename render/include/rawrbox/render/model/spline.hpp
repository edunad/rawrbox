#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/model/base.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class Spline : public rawrbox::ModelBase<M> {
		rawrbox::BezierCurve _curve = {};

	public:
		explicit Spline() = default;
		Spline(const Spline&) = delete;
		Spline(Spline&&) = delete;
		Spline& operator=(const Spline&) = delete;
		Spline& operator=(Spline&&) = delete;
		~Spline() override = default;

		void setPoints(const std::vector<rawrbox::Vector3f>& points) {
			this->_curve.setPoints(points);
		}

		void draw() override {
			/*ModelBase<M>::draw();
			this->_material->process(*this->_mesh); // Set atlas

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh);
				bgfx::setIndexBuffer(this->_ibdh);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh);
				bgfx::setIndexBuffer(this->_ibh);
			}

			bgfx::setState(BGFX_STATE_DEFAULT, 0);
			this->_material->postProcess();*/
		}
	};
} // namespace rawrbox
