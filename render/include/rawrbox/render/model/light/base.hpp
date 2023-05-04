#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrBox {

	enum LightType {
		LIGHT_UNKNOWN = 0,

		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_DIR,
	};

	class LightBase {
	protected:
		bool _isOn = true;

		rawrBox::Vector3f _posMatrix;
		rawrBox::Colorf _diffuse = rawrBox::Colors::White;
		rawrBox::Colorf _specular = rawrBox::Colors::White;
		rawrBox::Colorf _ambient = rawrBox::Colors::White;

	public:
		LightBase(rawrBox::Vector3f posMatrix, rawrBox::Colorf diffuse, rawrBox::Colorf specular) : _posMatrix(posMatrix), _diffuse(diffuse), _specular(specular){};
		LightBase(LightBase&&) = delete;
		LightBase& operator=(LightBase&&) = delete;
		LightBase(const LightBase&) = delete;
		LightBase& operator=(const LightBase&) = delete;

		virtual ~LightBase() = default;

		[[nodiscard]] virtual const rawrBox::Colorf& getSpecularColor() const { return this->_specular; }
		[[nodiscard]] virtual const rawrBox::Colorf& getDiffuseColor() const { return this->_diffuse; }
		[[nodiscard]] virtual const rawrBox::Colorf& getAmbientColor() const { return this->_ambient; }

		[[nodiscard]] virtual const bool isOn() const { return this->_isOn; }
		virtual void setStatus(bool on) { this->_isOn = on; };

		virtual std::array<float, 4> getPosMatrix() { return {this->_posMatrix.x, this->_posMatrix.y, this->_posMatrix.z, 0}; }
		virtual std::array<float, 16> const getDataMatrix() = 0;

		virtual LightType getType() = 0;
	};
} // namespace rawrBox
