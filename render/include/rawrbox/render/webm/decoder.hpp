#pragma once

#include <rawrbox/math/vector2.hpp>

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
		std::vector<uint8_t> buffer = {};
		rawrbox::VIDEO_CODEC codec = rawrbox::VIDEO_CODEC::UNKNOWN;

		inline bool valid() { return buffer.size() > 0; }
	};

	struct WEBMImage {
		std::vector<unsigned char> pixels = {};
		rawrbox::Vector2i size = {};

		inline bool valid() { return pixels.size() > 0; }
	};

	class WEBMDecoder {
	private:
		static rawrbox::VIDEO_CODEC _codec;
		static std::unique_ptr<vpx_codec_ctx> _ctx;
		static const void* _iter;

	public:
		static void init(uint32_t threads = 3, rawrbox::VIDEO_CODEC codec = rawrbox::VIDEO_CODEC::VIDEO_VP9);
		static void shutdown();

		static bool decode(const rawrbox::WEBMFrame& frame);
		static rawrbox::WEBMImage getImageFrame();
	};
} // namespace rawrbox
