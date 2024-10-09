#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/webp.hpp>

#include <webp/decode.h>
#include <webp/demux.h>

// #include <webp/encode.h>
// #include <webp/mux.h>

namespace rawrbox {
	std::unique_ptr<rawrbox::Logger> WEBP::_logger = std::make_unique<rawrbox::Logger>("RawrBox-WEBP");

	rawrbox::ImageData WEBP::decode(const uint8_t* buffer, size_t bufferSize) {
		if (buffer == nullptr || bufferSize <= 0) CRITICAL_RAWRBOX("Invalid data, cannot be empty!");

		WebPAnimDecoderOptions options;
		options.use_threads = 1;
		if (WebPAnimDecoderOptionsInit(&options) == 0) CRITICAL_RAWRBOX("Error initializing image!");

		WebPData webp_data = {buffer, bufferSize};

		auto* decoder = WebPAnimDecoderNew(&webp_data, &options);
		if (decoder == nullptr) CRITICAL_RAWRBOX("Error initializing decoder!");

		WebPAnimInfo info;
		if (WebPAnimDecoderGetInfo(decoder, &info) == 0) {
			WebPAnimDecoderDelete(decoder);
			CRITICAL_RAWRBOX("Error decoding image info!");
		}

		rawrbox::ImageData webpData = {};
		webpData.channels = 4;
		webpData.size = {info.canvas_width, info.canvas_height};
		webpData.frames.reserve(info.frame_count);

		float prevTime = 0.F;
		while (WebPAnimDecoderHasMoreFrames(decoder) != 0) {
			uint8_t* buf = nullptr;
			int delay = 0;

			if (WebPAnimDecoderGetNext(decoder, &buf, &delay) == 0) {
				WebPAnimDecoderDelete(decoder);
				CRITICAL_RAWRBOX("Error decoding image!");
			}

			rawrbox::ImageFrame frame = {};
			frame.delay = static_cast<float>(delay) - prevTime;
			frame.pixels.resize(info.canvas_width * info.canvas_height * 4);
			std::memcpy(frame.pixels.data(), buf, info.canvas_width * info.canvas_height * 4);

			webpData.frames.emplace_back(frame);
			prevTime = static_cast<float>(delay);
		}

		WebPAnimDecoderDelete(decoder);
		return webpData;
	}

	rawrbox::ImageData WEBP::decode(const std::vector<uint8_t>& data) {
		return decode(data.data(), data.size());
	}

	std::vector<uint8_t> WEBP::encode(const rawrbox::ImageData& /*data*/) {
		CRITICAL_RAWRBOX("Encoding WebP is not supported yet!");
		/*if (data.frames.empty() || data.size.length() <= 0.F) CRITICAL_RAWRBOX("Invalid webpData, frames cannot be empty!");

		int loop_count = 0; // infinite
		int timems_per_frame = 33;

		WebPConfig config;
		if (WebPConfigInit(&config) == 0) CRITICAL_RAWRBOX("Error initializing WebP config!");

		config.quality = 100; // Set quality to 100
		config.lossless = 1;  // Set lossless encoding

		WebPPicture picture;
		if (WebPPictureInit(&picture) == 0) CRITICAL_RAWRBOX("Error initializing WebP picture!");

		picture.width = data.size.x;
		picture.height = data.size.y;

		if (WebPPictureAlloc(&picture) == 0) {
			WebPPictureFree(&picture);
			CRITICAL_RAWRBOX("Error allocating WebP picture!");
		}

		WebPAnimEncoder* encoder = nullptr;
		WebPAnimEncoderOptions enc_options = {};

		if (WebPAnimEncoderOptionsInit(&enc_options) == 0) {
			WebPPictureFree(&picture);
			CRITICAL_RAWRBOX("Error initializing WebP animation encoder options!");
		}

		for (size_t i = 0; i < data.frames.size(); i++) {
			const auto& frame = data.frames[i];

			if (WebPPictureImportRGBA(&picture, frame.pixels.data(), static_cast<int>(frame.pixels.size() / 4)) == 0) {
				WebPPictureFree(&picture);
				CRITICAL_RAWRBOX("Error importing RGBA data to WebP picture!");
			}

			// if (colorspace == 0) WebPPictureARGBToYUVA(&pic, WebPEncCSP::WEBP_YUV420);
			// if (colorspace == 1) WebPPictureSharpARGBToYUVA(&pic);

			WebPPictureSharpARGBToYUVA(&picture);

			if (WebPAnimEncoderAdd(encoder, &picture, timems_per_frame * static_cast<int>(i), &config) == 0) {
				WebPPictureFree(&picture);
				CRITICAL_RAWRBOX("Error adding frame to WebP animation encoder!");
			}
		}

		WebPData webpData = {};
		WebPDataInit(&webpData);

		if (WebPAnimEncoderAssemble(encoder, &webpData) == 0) {
			WebPPictureFree(&picture);
			CRITICAL_RAWRBOX("Error assembling WebP animation!");
		}

		// Mux assemble
		WebPMux* mux = WebPMuxCreate(&webpData, 1);

		WebPMuxAnimParams anim_params;
		anim_params.loop_count = loop_count;

		WebPMuxSetAnimationParams(mux, &anim_params);
		WebPMuxAssemble(mux, &webpData);

		std::vector<uint8_t> encodedData(webpData.size);
		std::memcpy(encodedData.data(), webpData.bytes, webpData.size);

		WebPPictureFree(&picture);
		WebPAnimEncoderDelete(encoder);
		WebPDataClear(&webpData);
		WebPMuxDelete(mux);

		return encodedData;*/
	}
} // namespace rawrbox
