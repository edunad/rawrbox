#pragma once

#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class TextureMissing : public rawrbox::TextureBase {
	public:
		std::vector<uint8_t> pixels;
		TextureMissing();
		TextureMissing(const TextureMissing& a) = default;

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGB8) override;
		// --------------------
	};
} // namespace rawrbox
