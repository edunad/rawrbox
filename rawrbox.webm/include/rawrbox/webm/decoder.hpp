#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <memory>
#include <vector>

struct vpx_codec_ctx;

namespace rawrbox {
	enum class VIDEO_CODEC {
		UNKNOWN = 0,
		VIDEO_VP8,
		VIDEO_VP9
	};

	struct WEBMFrame {
		long long pos = 0;

		std::vector<uint8_t> buffer = {};
		rawrbox::VIDEO_CODEC codec = rawrbox::VIDEO_CODEC::UNKNOWN;

		[[nodiscard]] inline bool valid() const { return !buffer.empty(); }
	};

	struct WEBMImage {
		std::vector<uint8_t> pixels = {};
		rawrbox::Vector2i size = {};

		[[nodiscard]] inline bool valid() const { return !pixels.empty(); }
	};

	class WEBMDecoder {
	private:
		static rawrbox::VIDEO_CODEC _codec;
		static std::unique_ptr<vpx_codec_ctx> _ctx;
		static const void* _iter;

		// LOGGER ------
		static std::unique_ptr<rawrbox::Logger> _logger;
		// -------------

	public:
		static void init(rawrbox::VIDEO_CODEC codec, uint32_t threads = 6);
		static void shutdown();

		static bool decode(const rawrbox::WEBMFrame& frame, rawrbox::WEBMImage& image);
	};
} // namespace rawrbox
