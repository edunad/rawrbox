#pragma once
#include <cstdint>

namespace rawrbox {
	enum class DrawPass : uint32_t {
		PASS_OPAQUE = 0,
		PASS_OVERLAY = 1
	};
}
