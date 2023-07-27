#pragma once

#include <rawrbox/math/vector2.hpp>

#include <cstdint>
#include <memory>

struct vpx_codec_ctx;

namespace rawrbox {

	struct WEBMFrame {
		uint32_t bufferSize;
		unsigned char* buffer;

		inline bool valid() { return bufferSize > 0; }
	};

	struct WEBMImage {
		unsigned char* pixels = nullptr;
		rawrbox::Vector2i size = {};
	};

	enum class WEBMImageSTATUS {
		OK,
		ERR_UNSUPPORTED,
		ERR_NO_FRAME
	};

	class WEBMDecoder {
	private:
		static std::unique_ptr<vpx_codec_ctx> _ctx;
		static const void* _iter;

	public:
		static void init();
		static void shutdown();

		static bool decode(const rawrbox::WEBMFrame& frame);
		static rawrbox::WEBMImageSTATUS getImageFrame(rawrbox::WEBMImage& image);
	};
} // namespace rawrbox
