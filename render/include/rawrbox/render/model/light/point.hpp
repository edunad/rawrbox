#pragma once
#ifdef RAWRBOX_DEBUG
	#include <rawrbox/debug/gizmos.hpp>
#endif
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
		LightPoint(rawrBox::Vector3f posMatrix, rawrBox::Colorf diffuse, rawrBox::Colorf specular, float constant, float linear, float quadratic) : rawrBox::LightBase(posMatrix, diffuse, specular), _constant(constant), _linear(linear), _quadratic(quadratic) {
#ifdef RAWRBOX_DEBUG
			GIZMOS::get().addLight(this);
#endif
		};

		LightPoint(LightPoint&&) = delete;
		LightPoint& operator=(LightPoint&&) = delete;
		LightPoint(const LightPoint&) = delete;
		LightPoint& operator=(const LightPoint&) = delete;
		~LightPoint() override {
#ifdef RAWRBOX_DEBUG
			GIZMOS::get().removeLight(this);
#endif
		};

		LightType getType() override { return LightType::LIGHT_POINT; };
		std::array<float, 16> const getDataMatrix() override {
			return {
			    this->_diffuse.r, this->_specular.r, 0, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, 0, 0,
			    this->_diffuse.b, this->_specular.b, 0, 0,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.F : 0};
		}
	};
} // namespace rawrBox
