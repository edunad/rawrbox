#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector3.hpp>

#include <array>

namespace rawrbox {

	enum LightType {
		LIGHT_UNKNOWN = 0,

		LIGHT_POINT,
		LIGHT_SPOT,
		LIGHT_DIR,
	};

	class LightBase {
	protected:
		bool _isOn = true;
		size_t _id = 0;

		rawrbox::Vector3f _posMatrix;
		rawrbox::Colorf _diffuse = rawrbox::Colors::White;
		rawrbox::Colorf _specular = rawrbox::Colors::White;
		rawrbox::Colorf _ambient = rawrbox::Colors::White;

	public:
		LightBase() = default;
		LightBase(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : _posMatrix(posMatrix), _diffuse(diffuse), _specular(specular){};

		LightBase(LightBase&&) = delete;
		LightBase& operator=(LightBase&&) = delete;
		LightBase(const LightBase&) = delete;
		LightBase& operator=(const LightBase&) = delete;
		virtual ~LightBase() = default;

		[[nodiscard]] virtual const rawrbox::Colorf& getSpecularColor() const { return this->_specular; }
		[[nodiscard]] virtual const rawrbox::Colorf& getDiffuseColor() const { return this->_diffuse; }
		[[nodiscard]] virtual const rawrbox::Colorf& getAmbientColor() const { return this->_ambient; }

		[[nodiscard]] virtual const size_t id() const { return this->_id; };
		virtual void setId(size_t id) { this->_id = id; };

		[[nodiscard]] virtual const bool isOn() const { return this->_isOn; }
		virtual void setStatus(bool on) { this->_isOn = on; };

		virtual std::array<float, 4> getPosMatrix() { return {this->_posMatrix.x, this->_posMatrix.y, this->_posMatrix.z, 0}; }
		virtual std::array<float, 16> const getDataMatrix() = 0;

		virtual LightType getType() = 0;
	};
} // namespace rawrbox
