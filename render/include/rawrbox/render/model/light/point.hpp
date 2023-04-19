#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightPoint : public rawrBox::LightBase {
	protected:
		float _constant;
		float _linear;
		float _quadratic;

	public:
		LightPoint(std::array<float, 16> posMatrix, rawrBox::Colorf diffuse, rawrBox::Colorf specular, float constant, float linear, float quadratic) : rawrBox::LightBase(posMatrix, diffuse, specular), _constant(constant), _linear(linear), _quadratic(quadratic){};

		virtual LightType getType() override { return LightType::LIGHT_POINT; };
		virtual std::array<float, 16> getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_specular.r, 0, 0,
			    this->_diffuse.g, this->_specular.g, 0, 0,
			    this->_diffuse.b, this->_specular.b, 0, 0,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.f : 0};
		}
	};
} // namespace rawrBox
