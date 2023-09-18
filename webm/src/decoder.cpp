
#include <rawrbox/math/utils/yuv.hpp>
#include <rawrbox/webm/decoder.hpp>

#include <magic_enum.hpp>

#include <fmt/format.h>
#include <vpx/vp8dx.h>
#include <vpx/vpx_decoder.h>

namespace rawrbox {
	// PRIVATE -----
	std::unique_ptr<vpx_codec_ctx> WEBMDecoder::_ctx = nullptr;
	const void* WEBMDecoder::_iter = nullptr;

	rawrbox::VIDEO_CODEC WEBMDecoder::_codec;
	// ------

	void WEBMDecoder::init(rawrbox::VIDEO_CODEC codec, uint32_t threads) {
		const vpx_codec_dec_cfg_t codecCfg = {
		    threads,
		    0,
		    0};

		vpx_codec_iface_t* codecIface = nullptr;
		switch (codec) {
			case rawrbox::VIDEO_CODEC::VIDEO_VP8:
				codecIface = vpx_codec_vp8_dx();
				break;
			case rawrbox::VIDEO_CODEC::VIDEO_VP9:
				codecIface = vpx_codec_vp9_dx();
				break;
			default:
				throw std::runtime_error("[WEBMDecoder] Invalid vpx codec");
		}

		_codec = codec;
		_ctx = std::make_unique<vpx_codec_ctx>();

		if (vpx_codec_dec_init(_ctx.get(), codecIface, &codecCfg, 0 | VPX_CODEC_USE_FRAME_THREADING)) {
			_ctx.reset();
			throw std::runtime_error("[WEBMDecoder] Failed to initialize vpx codec");
		}
	}

	void WEBMDecoder::shutdown() {
		vpx_codec_destroy(_ctx.get());
		_ctx.reset();
	}

	bool WEBMDecoder::decode(const rawrbox::WEBMFrame& frame, rawrbox::WEBMImage& image) {
		if (_ctx == nullptr)
			throw std::runtime_error("[WEBMDecoder] Codec not initialized, did you call 'init' ?");

		if (frame.codec != _codec) {
			auto badname = magic_enum::enum_name(static_cast<rawrbox::VIDEO_CODEC>(frame.codec)).data();
			auto name = magic_enum::enum_name(static_cast<rawrbox::VIDEO_CODEC>(_codec)).data();

			throw std::runtime_error(fmt::format("[WEBMDecoder] Codec '{}' not set as config! '{}' was loaded instead", badname, name));
		}

		_iter = nullptr;
		if (vpx_codec_decode(_ctx.get(), frame.buffer.data(), static_cast<uint32_t>(frame.buffer.size()), nullptr, 0)) return false;

		if (vpx_image_t* img = vpx_codec_get_frame(_ctx.get(), &_iter)) {
			if ((img->fmt & VPX_IMG_FMT_PLANAR) == 0) throw std::runtime_error("[WEBMDecoder] Failed to get image! Image not in FMT_PLANAR!");

			rawrbox::YUVLuminanceScale scale = rawrbox::YUVLuminanceScale::UNKNOWN;
			int channels = 4;

			image.size = {static_cast<int>(img->d_w), static_cast<int>(img->d_h)};
			image.pixels.resize(image.size.x * image.size.y * channels);

			switch (img->range) {
				case VPX_CR_STUDIO_RANGE:
					scale = rawrbox::YUVLuminanceScale::ITU;
					break;
				case VPX_CR_FULL_RANGE:
					scale = rawrbox::YUVLuminanceScale::FULL;
					break;
				default:
					throw std::runtime_error("[WEBMDecoder] Unknown luminance format");
			}

			switch (img->fmt) {
				case VPX_IMG_FMT_I420:
					if ((img->fmt & VPX_IMG_FMT_HAS_ALPHA) != 0) {
						rawrbox::YUVUtils::convert420(scale, image.pixels.data(), image.size.x * channels, img->planes[0], img->planes[1], img->planes[2], img->planes[3], img->d_w, img->d_h, img->stride[0], img->stride[1]);
					} else {
						rawrbox::YUVUtils::convert420(scale, image.pixels.data(), image.size.x * channels, img->planes[0], img->planes[1], img->planes[2], img->d_w, img->d_h, img->stride[0], img->stride[1]);
					}
					break;
				default:
					throw std::runtime_error("[WEBMDecoder] Format not supported, video not in I420 format");
			}
		}

		return true;
	}
} // namespace rawrbox
