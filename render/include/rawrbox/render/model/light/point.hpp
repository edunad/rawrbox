#pragma once
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightPoint : public rawrbox::LightBase {
	protected:
	public:
		LightPoint(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular, float constant, float linear, float quadratic) : rawrbox::LightBase(posMatrix, diffuse, specular) {
			_constant = constant;
			_linear = linear;
			_quadratic = quadratic;
		};

		LightPoint(const LightPoint&) = default;
		LightPoint(LightPoint&&) = delete;
		LightPoint& operator=(const LightPoint&) = default;
		LightPoint& operator=(LightPoint&&) = delete;
		~LightPoint() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_POINT; };
		[[nodiscard]] const rawrbox::Matrix4x4 getDataMatrix() const override {
			return rawrbox::Matrix4x4({this->_diffuse.r, this->_specular.r, 0.F, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, 0.F, 0.F,
			    this->_diffuse.b, this->_specular.b, 0.F, 0.F,
			    this->_constant, this->_linear, this->_quadratic, this->_isOn ? 1.F : 0.F});
		}
	};
} // namespace rawrbox
