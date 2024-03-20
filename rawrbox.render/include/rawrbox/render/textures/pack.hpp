#pragma once

#include <rawrbox/render/textures/base.hpp>

#include <optional>

namespace rawrbox {
	struct PackNode {
		uint32_t x = 0;
		uint32_t y = 0;
		uint32_t width = 0;
		uint32_t height = 0;

		std::vector<uint8_t> data = {};

		std::unique_ptr<PackNode> left = nullptr;
		std::unique_ptr<PackNode> right = nullptr;

		bool canInsertNode(uint32_t insertedWidth, uint32_t insertedHeight);
		std::optional<std::reference_wrapper<rawrbox::PackNode>> InsertNode(uint32_t width, uint32_t height);

		PackNode() = default;
		PackNode(uint32_t _x, uint32_t _y, uint32_t _w, uint32_t _h) : x(_x), y(_y), width(_w), height(_h){};
	};

	class TexturePack : public rawrbox::TextureBase {
	private:
		size_t _spriteCount = 0;
		std::unique_ptr<rawrbox::PackNode> _root = nullptr;
		bool _pendingUpdate = false;

	public:
		explicit TexturePack(uint32_t size = 1024U);

		TexturePack(const TexturePack&) = delete;
		TexturePack(TexturePack&&) = delete;
		TexturePack& operator=(const TexturePack&) = delete;
		TexturePack& operator=(TexturePack&&) = delete;
		~TexturePack() override;

		[[nodiscard]] size_t getSpriteCount() const;

		bool canInsertNode(uint32_t width, uint32_t height);
		rawrbox::PackNode& addSprite(uint32_t width, uint32_t height, const std::vector<uint8_t>& data);

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
	};
} // namespace rawrbox
