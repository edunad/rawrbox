#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/base.hpp>
#include <rawrbox/render/textures/utils/ktx2.hpp>

#include <basisu_transcoder.h>

#include <algorithm>
#include <array>
#include <mutex>
#include <stdexcept>

namespace rawrbox {
	rawrbox::ImageData KTX2::decode(const uint8_t* buffer, size_t bufferSize) {
		// INIT ---
		static std::once_flag initialized;
		static bool bcSupported = false;

		std::call_once(initialized, []() {
			basist::basisu_transcoder_init();

			auto* device = rawrbox::RENDERER->device();
			bcSupported = std::ranges::all_of(std::array{
							      Diligent::TEX_FORMAT_BC1_UNORM,
							      Diligent::TEX_FORMAT_BC3_UNORM,
							      Diligent::TEX_FORMAT_BC7_UNORM},
			    [device](const Diligent::TEXTURE_FORMAT format) {
				    return (device->GetTextureFormatInfoExt(format).BindFlags & Diligent::BIND_SHADER_RESOURCE) != 0;
			    });
		});

		if (!bcSupported) throw std::runtime_error("GPU does not support BC compressed textures");
		// -----

		basist::ktx2_transcoder transcoder;
		if (!transcoder.init(buffer, static_cast<uint32_t>(bufferSize))) throw std::runtime_error("Invalid KTX2 file");

		if (transcoder.is_hdr()) throw std::runtime_error("HDR KTX2 textures are not supported"); // TODO: HDR support
		if (transcoder.get_faces() > 1U) throw std::runtime_error("Cubemap KTX2 textures are not supported");
		if (!transcoder.start_transcoding()) throw std::runtime_error("Failed to start KTX2 transcoding");

		// FORMAT ---
		const bool alpha = transcoder.get_has_alpha() != 0U;

		basist::transcoder_texture_format target = basist::transcoder_texture_format::cTFBC7_RGBA;
		Diligent::TEXTURE_FORMAT format = Diligent::TEX_FORMAT_BC7_UNORM;

		if (transcoder.is_etc1s()) {
			target = alpha ? basist::transcoder_texture_format::cTFBC3_RGBA : basist::transcoder_texture_format::cTFBC1_RGB;
			format = alpha ? Diligent::TEX_FORMAT_BC3_UNORM : Diligent::TEX_FORMAT_BC1_UNORM;
		}
		// -----

		basist::ktx2_image_level_info info = {};
		if (!transcoder.get_image_level_info(info, 0, 0, 0)) throw std::runtime_error("Failed to read KTX2 level info");

		// SIZE (power of 4) ---
		rawrbox::ImageData data = {};
		data.size = {(info.m_orig_width + 3U) & ~3U, (info.m_orig_height + 3U) & ~3U};
		data.channels = alpha ? 4U : 3U;
		data.format = format;
		// -----

		const uint32_t bytesPerBlock = basist::basis_get_bytes_per_block_or_pixel(target);
		const uint32_t layers = std::max(transcoder.get_layers(), 1U);

		for (uint32_t layer = 0; layer < layers; layer++) {
			rawrbox::ImageFrame frame = {};

			for (uint32_t level = 0; level < transcoder.get_levels(); level++) {
				if (!transcoder.get_image_level_info(info, level, layer, 0))
					throw std::runtime_error("Failed to read KTX2 level info");

				const uint32_t blocksX = (std::max(data.size.x >> level, 1U) + 3U) / 4U;
				const uint32_t blocksY = (std::max(data.size.y >> level, 1U) + 3U) / 4U;

				std::vector<uint8_t> blocks(static_cast<size_t>(blocksX) * blocksY * bytesPerBlock);
				if (!transcoder.transcode_image_level(level, layer, 0, blocks.data(), blocksX * blocksY, target, 0, blocksX))
					throw std::runtime_error("Failed to transcode KTX2 level");

				if (level == 0U) {
					frame.pixels = std::move(blocks);
				} else {
					frame.mips.push_back(std::move(blocks));
				}
			}

			data.frames.push_back(std::move(frame));
		}

		return data;
	}
} // namespace rawrbox