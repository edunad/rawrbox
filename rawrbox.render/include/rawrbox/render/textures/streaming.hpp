#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureStreaming : public rawrbox::TextureBase {
	protected:
		bool _pendingUpdate = true;
		bool _hasData = false;

	public:
		TextureStreaming(const rawrbox::Vector2u& size);

		// UTILS ---
		void clear();
		void setImage(const rawrbox::ImageData& data);
		[[nodiscard]] bool hasData() const;
		// --------

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;

		[[nodiscard]] uint32_t getTextureID() const override;

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
	};
} // namespace rawrbox
