#pragma once

#include <cstdint>

namespace rawrbox {
	struct ImageData;

	class KTX2 {
	public:
		static rawrbox::ImageData decode(const uint8_t* buffer, size_t bufferSize);
	};
} // namespace rawrbox