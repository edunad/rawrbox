#include <rawrbox/render/lights/point.hpp>

namespace rawrbox {
	rawrbox::LightType PointLight::getType() const { return LightType::POINT; };
} // namespace rawrbox
