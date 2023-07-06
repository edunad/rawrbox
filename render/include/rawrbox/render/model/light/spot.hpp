#pragma once
#include <rawrbox/render/model/light/base.hpp>

namespace rawrbox {
	class LightSpot : public rawrbox::LightBase {
	protected:
		rawrbox::Vector3 _direction;

		float _innerCone;
		float _outerCone;

	public:
		LightSpot(rawrbox::Vector3f posMatrix, rawrbox::Vector3f direction, rawrbox::Colorf diffuse, rawrbox::Colorf specular, float innerCone, float outerCone, float constant, float linear, float quadratic) : rawrbox::LightBase(posMatrix, diffuse, specular), _direction(direction), _innerCone(innerCone), _outerCone(outerCone) {
			this->_constant = constant;
			this->_linear = linear;
			this->_quadratic = quadratic;
		};

		LightSpot(const LightSpot&) = default;
		LightSpot(LightSpot&&) = delete;
		LightSpot& operator=(const LightSpot&) = default;
		LightSpot& operator=(LightSpot&&) = delete;
		~LightSpot() override = default;

		[[nodiscard]] const rawrbox::LightType getType() const override { return LightType::LIGHT_SPOT; };

		[[nodiscard]] const float getInnerCone() const { return _innerCone; };
		[[nodiscard]] const float getOuterCone() const { return _outerCone; };
		[[nodiscard]] const rawrbox::Vector3f& getDirection() const { return _direction; };
	};
} // namespace rawrbox
