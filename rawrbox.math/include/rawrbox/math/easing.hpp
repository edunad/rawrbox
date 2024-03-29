#pragma once

namespace rawrbox {

	enum class Easing {
		LINEAR,
		STEP,
		EASE_IN_QUAD,
		EASE_OUT_QUAD,
		EASE_IN_OUT_QUAD,
		EASE_IN_CUBIC,
		EASE_OUT_CUBIC,
		EASE_IN_OUT_CUBIC,
	};

	class EasingUtils {
	public:
		static float ease(rawrbox::Easing easing, float val);
	};
} // namespace rawrbox
