#pragma once

#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <memory>
#include <vector>

namespace rawrbox {
	struct ImageData;
	class WEBP {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static rawrbox::ImageData decode(const uint8_t* buffer, size_t bufferSize);
		static rawrbox::ImageData decode(const std::vector<uint8_t>& data);

		static std::vector<uint8_t> encode(const rawrbox::ImageData& data);
	};
} // namespace rawrbox
