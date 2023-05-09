#pragma once

#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
		#include <rawrbox/debug/gizmos.hpp>
	#endif
#endif

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightPoint : public rawrbox::LightBase {
	protected:
		float _constant;
		float _linear;
		float _quadratic;

	public:
		LightPoint(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular, float constant, float linear, float quadratic) : rawrbox::LightBase(posMatrix, diffuse, specular), _constant(constant), _linear(linear), _quadratic(quadratic) {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().addLight(this);
	#endif
#endif
		};

		LightPoint(LightPoint&&) = delete;
		LightPoint& operator=(LightPoint&&) = delete;
		LightPoint(const LightPoint&) = delete;
		LightPoint& operator=(const LightPoint&) = delete;
		~LightPoint() override {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().removeLight(this);
	#endif
#endif
		};

		LightType getType() override { return LightType::LIGHT_POINT; };
		rawrbox::Matrix4x4 const getDataMatrix() override {
			return rawrbox::Matrix4x4({this->_diffuse.r, this->_specular.r, 0.F, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, 0.F, 0.F,
			    this->_diffuse.b, this->_specular.b, 0.F, 0.F,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.F : 0.F});
		}
	};
} // namespace rawrbox
