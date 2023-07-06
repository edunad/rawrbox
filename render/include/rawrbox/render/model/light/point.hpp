#pragma once
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightPoint : public rawrbox::LightBase {
	protected:
	public:
		LightPoint(rawrbox::Vector3f posMatrix, rawrbox::Colorf diffuse, rawrbox::Colorf specular, float constant, float linear, float quadratic) : rawrbox::LightBase(posMatrix, diffuse, specular) {
			this->_constant = constant;
			this->_linear = linear;
			this->_quadratic = quadratic;
		};

		LightPoint(const LightPoint&) = default;
		LightPoint(LightPoint&&) = delete;
		LightPoint& operator=(const LightPoint&) = default;
		LightPoint& operator=(LightPoint&&) = delete;
		~LightPoint() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_POINT; };
	};
} // namespace rawrbox
