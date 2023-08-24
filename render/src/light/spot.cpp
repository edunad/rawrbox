#include <rawrbox/render/light/spot.hpp>

namespace rawrbox {
	SpotLight::SpotLight(const rawrbox::Vector3f& posMatrix, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float innerCone, float outerCone, float power) : rawrbox::LightBase(posMatrix, color, power), _innerCone(innerCone), _outerCone(outerCone) {
		this->_direction = direction.normalized();
	};

	rawrbox::LightType SpotLight::getType() const { return LightType::SPOT; };
	const rawrbox::Vector4f SpotLight::getData() const { return {_innerCone, _outerCone, 0, 0}; }

	float SpotLight::getInnerCone() const { return _innerCone; };
	float SpotLight::getOuterCone() const { return _outerCone; };

} // namespace rawrbox
