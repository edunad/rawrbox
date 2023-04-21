#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightSpot : public rawrBox::LightBase {
	protected:
		rawrBox::Vector3 _direction;

		float _innerCone;
		float _outerCone;

		float _constant;
		float _linear;
		float _quadratic;

	public:
		LightSpot(rawrBox::Vector3f posMatrix, rawrBox::Vector3f direction, rawrBox::Colorf diffuse, rawrBox::Colorf specular, float innerCone, float outerCone, float constant, float linear, float quadratic) : rawrBox::LightBase(posMatrix, diffuse, specular), _direction(direction), _innerCone(innerCone), _outerCone(outerCone), _constant(constant), _linear(linear), _quadratic(quadratic){};

		virtual LightType getType() override { return LightType::LIGHT_SPOT; };
		virtual std::array<float, 16> getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_specular.r, this->_direction.x, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, this->_direction.y, this->_innerCone,
			    this->_diffuse.b, this->_specular.b, this->_direction.z, this->_outerCone,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.f : 0};
		}
	};
} // namespace rawrBox
