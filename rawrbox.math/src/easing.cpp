#include <rawrbox/math/easing.hpp>

#include <stdexcept>

namespace rawrbox {
	float EasingUtils::ease(rawrbox::Easing ease, float in) {
		switch (ease) {
			case Easing::LINEAR:
				return in;
			case Easing::STEP:
				return in < 0.5F ? 0.F : 1.F;
			default:
				throw std::runtime_error("[RawrBox-Easing] Unsupported easing");
		}
	}
} // namespace rawrbox
