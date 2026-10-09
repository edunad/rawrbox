#pragma once

#include <rawrbox/math/utils/yuv.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/utils/logger.hpp>

#include <cstdint>
#include <memory>
#include <vector>

struct vpx_codec_ctx;
struct vpx_image;

namespace rawrbox {
	enum class VIDEO_CODEC {
		UNKNOWN = 0,
		VIDEO_VP8,
		VIDEO_VP9
	};

	struct WEBMFrame {
		std::vector<uint8_t> buffer = {};
		std::vector<uint8_t> alpha = {};

		[[nodiscard]] bool valid() const;
		[[nodiscard]] bool hasAlpha() const;
	};

	struct WEBMImage {
		std::vector<uint8_t> pixels = {};
		rawrbox::Vector2u size = {};

		[[nodiscard]] bool valid() const;
	};

	struct WEBMColorHint {
		rawrbox::YUVColorSpace space = rawrbox::YUVColorSpace::UNKNOWN;
		rawrbox::YUVLuminanceScale scale = rawrbox::YUVLuminanceScale::UNKNOWN;
	};

	class WEBMDecoder {
	private:
		rawrbox::VIDEO_CODEC _codec = rawrbox::VIDEO_CODEC::UNKNOWN;
		rawrbox::WEBMColorHint _hint = {};
		
		uint32_t _threads = 1;

		std::unique_ptr<vpx_codec_ctx> _ctx;
		std::unique_ptr<vpx_codec_ctx> _alphaCtx;

		vpx_image* _image = nullptr;
		vpx_image* _alphaImage = nullptr;

		// LOGGER ------
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-WEBMDecoder");
		// -------------

		void createContext(std::unique_ptr<vpx_codec_ctx>& ctx) const;
		void destroyContext(std::unique_ptr<vpx_codec_ctx>& ctx) const;
		[[nodiscard]] vpx_image* decodeStream(vpx_codec_ctx* ctx, const std::vector<uint8_t>& buffer) const;

		[[nodiscard]] rawrbox::YUVColorSpace getColorSpace() const;
		[[nodiscard]] rawrbox::YUVLuminanceScale getLuminanceScale() const;

	public:
		explicit WEBMDecoder(rawrbox::VIDEO_CODEC codec, const rawrbox::WEBMColorHint& hint = {}, uint32_t threads = 6);
		WEBMDecoder(const WEBMDecoder&) = delete;
		WEBMDecoder(WEBMDecoder&&) = delete;
		WEBMDecoder& operator=(const WEBMDecoder&) = delete;
		WEBMDecoder& operator=(WEBMDecoder&&) = delete;
		~WEBMDecoder();

		[[nodiscard]] bool decode(const rawrbox::WEBMFrame& frame);
		void convert(rawrbox::WEBMImage& image) const;

		[[nodiscard]] rawrbox::VIDEO_CODEC getCodec() const;
	};
} // namespace rawrbox
