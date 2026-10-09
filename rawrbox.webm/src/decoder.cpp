
#include <rawrbox/webm/decoder.hpp>

#include <fmt/format.h>
#include <vpx/vp8dx.h>
#include <vpx/vpx_decoder.h>

#include <algorithm>
#include <string>
#include <thread>

namespace rawrbox {
	// FRAME ------
	bool WEBMFrame::valid() const { return !this->buffer.empty(); }
	bool WEBMFrame::hasAlpha() const { return !this->alpha.empty(); }
	// -------------

	// IMAGE ------
	bool WEBMImage::valid() const { return !this->pixels.empty(); }
	// -------------

	WEBMDecoder::WEBMDecoder(rawrbox::VIDEO_CODEC codec, const rawrbox::WEBMColorHint& hint, uint32_t threads) : _codec(codec), _hint(hint) {
		const uint32_t cores = std::thread::hardware_concurrency();
		this->_threads = std::max(1U, cores == 0U ? threads : std::min(threads, cores));

		this->createContext(this->_ctx, this->_threads);
	}

	WEBMDecoder::~WEBMDecoder() {
		this->_image = nullptr;
		this->_alphaImage = nullptr;

		this->destroyContext(this->_alphaCtx);
		this->destroyContext(this->_ctx);
	}

	// PRIVATE ------
	void WEBMDecoder::createContext(std::unique_ptr<vpx_codec_ctx>& ctx, uint32_t threads) const {
		vpx_codec_iface_t* codecIface = nullptr;

		switch (this->_codec) {
			case rawrbox::VIDEO_CODEC::VIDEO_VP8:
				codecIface = vpx_codec_vp8_dx();
				break;
			case rawrbox::VIDEO_CODEC::VIDEO_VP9:
				codecIface = vpx_codec_vp9_dx();
				break;
			default:
				RAWRBOX_CRITICAL("Invalid vpx codec");
		}

		vpx_codec_dec_cfg_t codecCfg = {};
		codecCfg.threads = threads;

		ctx = std::make_unique<vpx_codec_ctx>();
		if (vpx_codec_dec_init(ctx.get(), codecIface, &codecCfg, 0) != VPX_CODEC_OK) {
			const std::string error = vpx_codec_error(ctx.get());
			ctx.reset();

			RAWRBOX_CRITICAL("Failed to initialize vpx codec: {}", error);
		}

		if (this->_codec == rawrbox::VIDEO_CODEC::VIDEO_VP9 && threads > 1U && vpx_codec_control(ctx.get(), VP9D_SET_ROW_MT, 1) != VPX_CODEC_OK) {
			this->_logger->warn("Failed to enable VP9 row multi-threading: {}", vpx_codec_error(ctx.get()));
		}
	}

	void WEBMDecoder::destroyContext(std::unique_ptr<vpx_codec_ctx>& ctx) const {
		if (ctx == nullptr) return;

		if (vpx_codec_destroy(ctx.get()) != VPX_CODEC_OK) {
			this->_logger->warn("Failed to destroy vpx codec: {}", vpx_codec_error(ctx.get()));
		}

		ctx.reset();
	}

	vpx_image* WEBMDecoder::decodeStream(vpx_codec_ctx* ctx, const std::vector<uint8_t>& buffer) const {
		if (vpx_codec_decode(ctx, buffer.data(), static_cast<unsigned int>(buffer.size()), nullptr, 0) != VPX_CODEC_OK) {
			const char* detail = vpx_codec_error_detail(ctx);

			this->_logger->warn("Failed to decode frame: {} {}", vpx_codec_error(ctx), detail == nullptr ? "" : detail);
			return nullptr;
		}

		vpx_codec_iter_t iter = nullptr;
		vpx_image* image = nullptr;

		while (vpx_image* next = vpx_codec_get_frame(ctx, &iter)) {
			image = next;
		}

		return image;
	}

	rawrbox::YUVColorSpace WEBMDecoder::getColorSpace() const {
		switch (this->_image->cs) {
			case VPX_CS_BT_601:
			case VPX_CS_SMPTE_170:
				return rawrbox::YUVColorSpace::BT601;
			case VPX_CS_BT_709:
				return rawrbox::YUVColorSpace::BT709;
			case VPX_CS_SMPTE_240:
				return rawrbox::YUVColorSpace::SMPTE240;
			case VPX_CS_BT_2020:
				return rawrbox::YUVColorSpace::BT2020;
			case VPX_CS_SRGB:
				return rawrbox::YUVColorSpace::RGB;
			default:
				break;
		}

		if (this->_hint.space != rawrbox::YUVColorSpace::UNKNOWN) return this->_hint.space;
		return this->_image->d_h >= 720U ? rawrbox::YUVColorSpace::BT709 : rawrbox::YUVColorSpace::BT601;
	}

	rawrbox::YUVLuminanceScale WEBMDecoder::getLuminanceScale() const {
		if (this->_codec == rawrbox::VIDEO_CODEC::VIDEO_VP8 && this->_hint.scale != rawrbox::YUVLuminanceScale::UNKNOWN) return this->_hint.scale;
		return this->_image->range == VPX_CR_FULL_RANGE ? rawrbox::YUVLuminanceScale::FULL : rawrbox::YUVLuminanceScale::ITU;
	}
	// -------------

	bool WEBMDecoder::decode(const rawrbox::WEBMFrame& frame) {
		if (this->_ctx == nullptr) RAWRBOX_CRITICAL("WEBM codec not initialized");

		this->_image = nullptr;
		this->_alphaImage = nullptr;

		if (!frame.valid()) return false;
		this->_image = this->decodeStream(this->_ctx.get(), frame.buffer);

		if (frame.hasAlpha()) {
			if (this->_alphaCtx == nullptr) this->createContext(this->_alphaCtx, 1U);
			this->_alphaImage = this->decodeStream(this->_alphaCtx.get(), frame.alpha);
		}

		return this->_image != nullptr;
	}

	void WEBMDecoder::convert(rawrbox::WEBMImage& image) const {
		if (this->_image == nullptr) return;

		const auto format = static_cast<vpx_img_fmt_t>(this->_image->fmt & ~VPX_IMG_FMT_HIGHBITDEPTH);
		if (format != VPX_IMG_FMT_I420 && format != VPX_IMG_FMT_I422 && format != VPX_IMG_FMT_I444 && format != VPX_IMG_FMT_I440) {
			RAWRBOX_CRITICAL("Unsupported vpx image format '{}'", static_cast<int>(this->_image->fmt));
		}

		rawrbox::YUVImage yuv = {};
		yuv.planes = {this->_image->planes[0], this->_image->planes[1], this->_image->planes[2], nullptr};
		yuv.strides = {this->_image->stride[0], this->_image->stride[1], this->_image->stride[2], 0};
		yuv.size = {this->_image->d_w, this->_image->d_h};

		yuv.chromaShift = {this->_image->x_chroma_shift, this->_image->y_chroma_shift};
		yuv.wide = (this->_image->fmt & VPX_IMG_FMT_HIGHBITDEPTH) != 0;
		yuv.bitDepth = yuv.wide ? this->_image->bit_depth : 8U;

		yuv.scale = this->getLuminanceScale();
		yuv.space = this->getColorSpace();

		const vpx_image* alpha = this->_alphaImage;
		if (alpha != nullptr && alpha->d_w == this->_image->d_w && alpha->d_h == this->_image->d_h) {
			yuv.planes[3] = alpha->planes[0];
			yuv.strides[3] = alpha->stride[0];

			yuv.wideAlpha = (alpha->fmt & VPX_IMG_FMT_HIGHBITDEPTH) != 0;
			yuv.alphaBitDepth = yuv.wideAlpha ? alpha->bit_depth : 8U;
		}

		image.size = yuv.size;
		image.pixels.resize(static_cast<size_t>(image.size.x) * image.size.y * 4U);

		try {
			rawrbox::YUVUtils::convert(yuv, image.pixels.data(), image.size.x * 4U, true);
		} catch (const std::exception& e) {
			RAWRBOX_CRITICAL("Failed to convert frame: {}", e.what());
		}
	}

	rawrbox::VIDEO_CODEC WEBMDecoder::getCodec() const { return this->_codec; }
} // namespace rawrbox
