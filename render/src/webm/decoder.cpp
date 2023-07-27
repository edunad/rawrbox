
#include <rawrbox/render/webm/decoder.hpp>

#include <fmt/format.h>
#include <vpx/vp8dx.h>
#include <vpx/vpx_decoder.h>

namespace rawrbox {
	// PRIVATE -----
	std::unique_ptr<vpx_codec_ctx> WEBMDecoder::_ctx = nullptr;
	const void* WEBMDecoder::_iter = nullptr;
	// ------

	void WEBMDecoder::init() {
		const vpx_codec_dec_cfg_t codecCfg = {
		    3,
		    0,
		    0};

		vpx_codec_iface_t* codecIface = vpx_codec_vp8_dx();
		_ctx = std::make_unique<vpx_codec_ctx>();

		if (vpx_codec_dec_init(_ctx.get(), codecIface, &codecCfg, VPX_CODEC_USE_FRAME_THREADING)) {
			_ctx.reset();
			throw std::runtime_error("[WEBMDecoder] Failed to initialize vpx codec");
		}
	}

	void WEBMDecoder::shutdown() {
		vpx_codec_destroy(_ctx.get());
		_ctx.reset();
	}

	bool WEBMDecoder::decode(const rawrbox::WEBMFrame& frame) {
		if (_ctx == nullptr)
			throw std::runtime_error("[WEBMDecoder] Codec not initialized, did you call 'init' ?");

		_iter = nullptr;
		return !vpx_codec_decode(_ctx.get(), frame.buffer, frame.bufferSize, nullptr, 0);
	}

	rawrbox::WEBMImageSTATUS WEBMDecoder::getImageFrame(rawrbox::WEBMImage& image) {
		rawrbox::WEBMImageSTATUS err = rawrbox::WEBMImageSTATUS::OK;

		if (vpx_image_t* img = vpx_codec_get_frame(_ctx.get(), &_iter)) {

			image.size = {11, 11};
			// It seems to be a common problem that UNKNOWN comes up a lot, yet FFMPEG is somehow getting accurate colour-space information.
			// After checking FFMPEG code, *they're* getting colour-space information, so I'm assuming something like this is going on.
			// It appears to work, at least.
			/*if (img->cs != VPX_CS_UNKNOWN)
				m_last_space = img->cs;

			if ((img->fmt & VPX_IMG_FMT_PLANAR) && !(img->fmt & (VPX_IMG_FMT_HAS_ALPHA | VPX_IMG_FMT_HIGHBITDEPTH))) {
				if (img->stride[0] && img->stride[1] && img->stride[2]) {
					const int uPlane = !!(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;
					const int vPlane = !(img->fmt & VPX_IMG_FMT_UV_FLIP) + 1;

					image.w = img->d_w;
					image.h = img->d_h;
					image.cs = m_last_space;
					image.chromaShiftW = img->x_chroma_shift;
					image.chromaShiftH = img->y_chroma_shift;

					image.planes[0] = img->planes[0];
					image.planes[1] = img->planes[uPlane];
					image.planes[2] = img->planes[vPlane];

					image.linesize[0] = img->stride[0];
					image.linesize[1] = img->stride[uPlane];
					image.linesize[2] = img->stride[vPlane];

					err = NO_ERROR;
				}
			} else {
				err = UNSUPPORTED_FRAME;
			}*/
		}

		return err;
	}
} // namespace rawrbox
