#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/light/base.hpp>

namespace rawrBox {
	class LightDirectional : public rawrBox::LightBase {
	protected:
		rawrBox::Vector3 _direction;

	public:
		LightDirectional(std::array<float, 16> posMatrix, rawrBox::Vector3 dir, rawrBox::Colorf diffuse, rawrBox::Colorf specular) : rawrBox::LightBase(posMatrix, diffuse, specular), _direction(dir){};

		virtual LightType getType() override { return LightType::LIGHT_DIR; };
		virtual std::array<float, 16> getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_specular.r, this->_direction.x, 0,
			    this->_diffuse.g, this->_specular.g, this->_direction.y, 0,
			    this->_diffuse.b, this->_specular.b, this->_direction.z, 0,
			    0, 0, 0, this->_isOn ? 1.f : 0};
		}
	};
} // namespace rawrBox
