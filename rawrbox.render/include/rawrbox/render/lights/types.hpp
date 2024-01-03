#pragma once

#include <cstdint>
namespace rawrbox {
	enum class LightType : uint32_t {
		UNKNOWN = 0,

		POINT,
		SPOT,
		DIR,
	};
}
