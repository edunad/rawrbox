#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrBox {

	enum LightType {
		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_DIR,
	};

	class LightBase {
	protected:
		bool _isOn = true;

		std::array<float, 16> _posMatrix;
		rawrBox::Colorf _diffuse = rawrBox::Colors::White;
		rawrBox::Colorf _specular = rawrBox::Colors::White;
		rawrBox::Colorf _ambient = rawrBox::Colors::White;

	public:
		uint32_t indx;

		virtual void setStatus(bool on) { this->_isOn = on; };

		LightBase(std::array<float, 16> posMatrix, rawrBox::Colorf diffuse, rawrBox::Colorf specular) : _posMatrix(posMatrix), _diffuse(diffuse), _specular(specular){};
		virtual ~LightBase() = default;

		virtual std::array<float, 16>& getPosMatrix() { return this->_posMatrix; }
		virtual std::array<float, 16> getDataMatrix() = 0;

		virtual LightType getType() = 0;
	};
} // namespace rawrBox
