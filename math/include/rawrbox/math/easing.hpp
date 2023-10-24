#pragma once

namespace rawrbox {

	enum class Easing {
		LINEAR = 0,
		STEP
	};

	class EasingUtils {
	public:
		static float ease(rawrbox::Easing easing, float val);
	};
} // namespace rawrbox
