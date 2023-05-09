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
	class LightSpot : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3 _direction;

		float _innerCone;
		float _outerCone;

		float _constant;
		float _linear;
		float _quadratic;

	public:
		LightSpot(rawrbox::Vector3f posMatrix, rawrbox::Vector3f direction, rawrbox::Colorf diffuse, rawrbox::Colorf specular, float innerCone, float outerCone, float constant, float linear, float quadratic) : rawrbox::LightBase(posMatrix, diffuse, specular), _direction(direction), _innerCone(innerCone), _outerCone(outerCone), _constant(constant), _linear(linear), _quadratic(quadratic) {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().addLight(this);
	#endif
#endif
		};

		LightSpot(LightSpot&&) = delete;
		LightSpot& operator=(LightSpot&&) = delete;
		LightSpot(const LightSpot&) = delete;
		LightSpot& operator=(const LightSpot&) = delete;
		~LightSpot() override {
#ifdef RAWRBOX_DEBUG
	#ifndef RAWRBOX_TESTING
			GIZMOS::get().removeLight(this);
	#endif
#endif
		};

		LightType getType() override { return LightType::LIGHT_SPOT; };
		rawrbox::Matrix4x4 const getDataMatrix() override {
			return rawrbox::Matrix4x4({this->_diffuse.r, this->_specular.r, this->_direction.x, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, this->_direction.y, this->_innerCone,
			    this->_diffuse.b, this->_specular.b, this->_direction.z, this->_outerCone,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.F : 0.F});
		}
	};
} // namespace rawrbox
