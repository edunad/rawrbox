#include <rawrbox/math/easing.hpp>

namespace rawrbox {
	float EasingUtils::ease(rawrbox::Easing ease, float in) {
		switch (ease) {
			case Easing::LINEAR:
				return in;
			case Easing::STEP:
				return in < 0.5F ? 0.F : 1.F;
		}
	}
} // namespace rawrbox
