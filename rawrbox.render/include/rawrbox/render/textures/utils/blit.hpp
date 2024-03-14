#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <RefCntAutoPtr.hpp>

#include <Fence.h>
#include <Texture.h>

#include <functional>

namespace rawrbox {
	class TextureBLIT : public rawrbox::TextureBase {
	protected:
		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-TextureBLIT");
		Diligent::RefCntAutoPtr<Diligent::IFence> _copyFence;

		std::vector<std::function<void()>> _callbacks = {};

	public:
		TextureBLIT(const rawrbox::Vector2u& size);
		TextureBLIT(TextureBLIT&&) = delete;
		TextureBLIT& operator=(TextureBLIT&&) = delete;
		TextureBLIT(const TextureBLIT&) = delete;
		TextureBLIT& operator=(const TextureBLIT&) = delete;
		~TextureBLIT() override;

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;

		void copy(Diligent::ITexture* base, Diligent::Box* box, const std::function<void()>& callback);
		void blit(Diligent::Box* box, const std::function<void(const uint8_t*, const uint64_t)>& callback);

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
	};
} // namespace rawrbox
