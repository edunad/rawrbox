#include <rawrbox/math/easing.hpp>

#include <cmath> // For std::pow
#include <stdexcept>

namespace rawrbox {
	float EasingUtils::ease(rawrbox::Easing easing, float val) {
		switch (easing) {
			case Easing::LINEAR:
				return val;
			case Easing::STEP:
				return val < 0.5F ? 0.F : 1.F;
			case Easing::EASE_IN_QUAD:
				return std::pow(val, 2);
			case Easing::EASE_OUT_QUAD:
				return 1 - std::pow(1 - val, 2);
			case Easing::EASE_IN_OUT_QUAD:
				return val < 0.5 ? 2 * std::pow(val, 2) : 1 - std::pow(-2 * val + 2, 2) / 2;
			case Easing::EASE_IN_CUBIC:
				return std::pow(val, 3);
			case Easing::EASE_OUT_CUBIC:
				return 1 - std::pow(1 - val, 3);
			case Easing::EASE_IN_OUT_CUBIC:
				return val < 0.5 ? 4 * std::pow(val, 3) : 1 - std::pow(-2 * val + 2, 3) / 2;
			default:
				throw std::runtime_error("[RawrBox-Easing] Unsupported easing");
		}
	}
} // namespace rawrbox
