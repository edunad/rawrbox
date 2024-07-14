#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureStreaming : public rawrbox::TextureBase {
	protected:
		int _tick = 0;
		bool _pendingUpdate = true;

		std::vector<uint8_t> _checker_1 = {};
		std::vector<uint8_t> _checker_2 = {};

	public:
		TextureStreaming(const rawrbox::Vector2u& size);

		void setPixelData(const std::vector<uint8_t>& data);

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
	};
} // namespace rawrbox
