#pragma once

#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureMissing : public TextureBase {
	public:
		std::vector<uint8_t> pixels;
		TextureMissing();

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGB8) override;
		// --------------------
	};
} // namespace rawrBox
