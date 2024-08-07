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
				return static_cast<float>(std::pow(val, 2.F));
			case Easing::EASE_OUT_QUAD:
				return 1.F - static_cast<float>(std::pow(1.F - val, 2.F));
			case Easing::EASE_IN_OUT_QUAD:
				return val < 0.5F ? 2.F * static_cast<float>(std::pow(val, 2.F)) : 1.F - static_cast<float>(std::pow(-2.F * val + 2.F, 2.F)) / 2.F;
			case Easing::EASE_IN_CUBIC:
				return static_cast<float>(std::pow(val, 3.F));
			case Easing::EASE_OUT_CUBIC:
				return 1.F - static_cast<float>(std::pow(1.F - val, 3.F));
			case Easing::EASE_IN_OUT_CUBIC:
				return val < 0.5F ? 4.F * static_cast<float>(std::pow(val, 3.F)) : 1.F - static_cast<float>(std::pow(-2.F * val + 2.F, 3.F)) / 2.F;
			default:
				throw std::runtime_error("[RawrBox-Easing] Unsupported easing");
		}
	}
} // namespace rawrbox
