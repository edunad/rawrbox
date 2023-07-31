#pragma once

#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <vector>

namespace rawrbox {
	class TextureMissing : public TextureBase {
	public:
		std::vector<uint8_t> pixels;
		TextureMissing();

		// ------RENDER
		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGB8) override;
		// --------------------
	};
} // namespace rawrbox
