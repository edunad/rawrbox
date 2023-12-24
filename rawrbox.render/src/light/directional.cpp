#include <rawrbox/render/light/directional.hpp>

namespace rawrbox {
	DirectionalLight::DirectionalLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color, float intensity) : rawrbox::LightBase(pos, color, intensity, 0) {
		this->_direction = direction.normalized();
	};

	rawrbox::LightType DirectionalLight::getType() const { return LightType::DIR; };
} // namespace rawrbox
