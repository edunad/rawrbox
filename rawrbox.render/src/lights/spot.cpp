#include <rawrbox/render/lights/spot.hpp>

namespace rawrbox {
	SpotLight::SpotLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float penumbra, float umbra, float radius) : rawrbox::LightBase(pos, color, radius), _umbra(umbra), _penumbra(penumbra) {
		this->_direction = direction.normalized();
	};

	rawrbox::LightType SpotLight::getType() const { return LightType::SPOT; };
	rawrbox::Vector4f SpotLight::getData() const { return {_penumbra, _umbra, 0, 0}; }

	float SpotLight::getUmbra() const { return _umbra; };
	float SpotLight::getPenumbra() const { return _penumbra; };

} // namespace rawrbox
