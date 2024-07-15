#pragma once

#include <rawrbox/render/textures/base.hpp>

namespace rawrbox {
	class TextureStreaming : public rawrbox::TextureBase {
	protected:
		bool _hasData = false;
		bool _pendingUpdate = true;

	public:
		TextureStreaming(const rawrbox::Vector2u& size);

		// UTILS ---
		void setImage(const rawrbox::ImageData& data);
		[[nodiscard]] bool hasData() const;
		// --------

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;
	};
} // namespace rawrbox
