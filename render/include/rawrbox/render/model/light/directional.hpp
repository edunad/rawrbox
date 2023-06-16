#pragma once
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightDirectional : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3 _direction;

	public:
		LightDirectional(rawrbox::Vector3f posMatrix, rawrbox::Vector3 dir, rawrbox::Colorf diffuse, rawrbox::Colorf specular) : rawrbox::LightBase(posMatrix, diffuse, specular), _direction(dir){};
		LightDirectional(const LightDirectional&) = default;
		LightDirectional(LightDirectional&&) = delete;
		LightDirectional& operator=(const LightDirectional&) = default;
		LightDirectional& operator=(LightDirectional&&) = delete;
		~LightDirectional() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_DIR; };
		[[nodiscard]] const rawrbox::Matrix4x4 getDataMatrix() const override {
			return rawrbox::Matrix4x4({this->_diffuse.r, this->_specular.r, this->_direction.x, static_cast<float>(this->getType()),
			    this->_diffuse.g, this->_specular.g, this->_direction.y, 0.F,
			    this->_diffuse.b, this->_specular.b, this->_direction.z, 0.F,
			    0.F, 0.F, 0.F, this->_isOn ? 1.F : 0.F});
		}
	};
} // namespace rawrbox
