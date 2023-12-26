#include <rawrbox/render/light/directional.hpp>

namespace rawrbox {
	DirectionalLight::DirectionalLight(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& direction, const rawrbox::Colorf& color) : rawrbox::LightBase(pos, color, 0) {
		this->_direction = direction.normalized();
	};

	rawrbox::LightType DirectionalLight::getType() const { return LightType::DIR; };
} // namespace rawrbox
