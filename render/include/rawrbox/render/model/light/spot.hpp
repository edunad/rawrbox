#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightSpot : public rawrBox::LightBase {
	protected:
		rawrBox::Vector3 _direction;

		float _cutOff;
		float _outerCutOff;

		// rawrBox::Colorf _diffuse;
		// rawrBox::Colorf _specular;

		float _constant;
		float _linear;
		float _quadratic;

	public:
		LightSpot(std::array<float, 16> posMatrix, rawrBox::Vector3f direction, rawrBox::Colorf diffuse, float cutOff, float outerCutOff, float constant, float linear, float quadratic) : rawrBox::LightBase(posMatrix, diffuse), _direction(direction), _cutOff(cutOff), _constant(constant), _outerCutOff(outerCutOff), _linear(linear), _quadratic(quadratic){};

		virtual LightType getType() override { return LightType::LIGHT_SPOT; };
		virtual std::array<float, 16> getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_direction.x, this->_cutOff, 0,
			    this->_diffuse.g, this->_direction.y, this->_outerCutOff, 0,
			    this->_diffuse.b, this->_direction.z, 0, 0,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.f : 0};
		}
	};
} // namespace rawrBox