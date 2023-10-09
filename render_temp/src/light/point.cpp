#include <rawrbox/render_temp/light/point.hpp>

namespace rawrbox {
	rawrbox::LightType PointLight::getType() const { return LightType::POINT; };
} // namespace rawrbox
