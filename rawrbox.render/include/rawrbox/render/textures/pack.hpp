#pragma once

#include <rawrbox/render/textures/base.hpp>

#include <optional>

namespace rawrbox {
	struct PackNode {
		uint16_t x = 0;
		uint16_t y = 0;
		uint16_t width = 0;
		uint16_t height = 0;

		std::vector<uint8_t> data = {};

		std::unique_ptr<PackNode> left = nullptr;
		std::unique_ptr<PackNode> right = nullptr;

		bool canInsertNode(uint16_t insertedWidth, uint16_t insertedHeight);
		std::optional<std::reference_wrapper<rawrbox::PackNode>> InsertNode(uint16_t width, uint16_t height);

		PackNode() = default;
		PackNode(uint16_t _x, uint16_t _y, uint16_t _w, uint16_t _h) : x(_x), y(_y), width(_w), height(_h){};
	};

	class TexturePack : public rawrbox::TextureBase {
	private:
		size_t _spriteCount = 0;
		std::unique_ptr<rawrbox::PackNode> _root = nullptr;
		bool _pendingUpdate = false;

	public:
		explicit TexturePack(uint16_t size = 1024U);

		TexturePack(const TexturePack&) = delete;
		TexturePack(TexturePack&&) = delete;
		TexturePack& operator=(const TexturePack&) = delete;
		TexturePack& operator=(TexturePack&&) = delete;
		~TexturePack() override;

		[[nodiscard]] size_t getSpriteCount() const;

		bool canInsertNode(uint16_t width, uint16_t height);
		rawrbox::PackNode& addSprite(uint16_t width, uint16_t height, const std::vector<uint8_t>& data);

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, bool dynamic = false) override;

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;
	};
} // namespace rawrbox
