#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <GPUCompletionAwaitQueue.hpp>
#include <RefCntAutoPtr.hpp>

#include <Texture.h>

#include <cstdint>
#include <deque>
#include <functional>
#include <optional>
#include <span>
#include <vector>

namespace rawrbox {
	class TextureBLIT : public rawrbox::TextureBase {
	public:
		// Horrible, i hate it
		using ReadCallback = std::function<void(std::span<const uint8_t> pixels, uint64_t stride, const rawrbox::Vector2u& size)>;

	protected:
		struct ReadRequest {
			Diligent::RefCntAutoPtr<Diligent::ITexture> source;
			Diligent::Box box = {};

			ReadCallback callback = nullptr;
		};

		std::unique_ptr<rawrbox::Logger> _logger = std::make_unique<rawrbox::Logger>("RawrBox-TextureBLIT");
		std::unique_ptr<Diligent::GPUCompletionAwaitQueue<Diligent::RefCntAutoPtr<Diligent::ITexture>>> _copies;
		bool _pending = false;
		bool _mapped = false;

		std::vector<std::function<void()>> _callbacks = {};
		std::deque<ReadRequest> _reads = {};

		virtual void createStaging();
		virtual void startRead(Diligent::ITexture& source, Diligent::Box box, ReadCallback callback);

	public:
		TextureBLIT(const rawrbox::Vector2u& size);
		TextureBLIT(TextureBLIT&&) = delete;
		TextureBLIT& operator=(TextureBLIT&&) = delete;
		TextureBLIT(const TextureBLIT&) = delete;
		TextureBLIT& operator=(const TextureBLIT&) = delete;
		~TextureBLIT() override;

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, uint32_t flags = rawrbox::TextureFlags::NONE) override;

		void resize(const rawrbox::Vector2u& size) override;

		void copy(Diligent::ITexture* base, Diligent::Box* box, const std::function<void()>& callback);

		void blit(Diligent::Box* box, const std::function<void(const uint8_t*, const uint64_t)>& callback);

		void read(const rawrbox::TextureBase& source, ReadCallback callback, std::optional<Diligent::Box> box = std::nullopt);
		void read(Diligent::ITexture& source, ReadCallback callback, std::optional<Diligent::Box> box = std::nullopt);

		[[nodiscard]] virtual bool isPending() const;

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
	};
} // namespace rawrbox