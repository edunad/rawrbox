#pragma once

#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <functional>
#include <memory>
#include <optional>

namespace rawrbox {
	struct PackNode {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		bool empty = true;

		std::unique_ptr<PackNode> left = nullptr;
		std::unique_ptr<PackNode> right = nullptr;

		bool canInsertNode(int insertedWidth, int insertedHeight);
		std::optional<std::reference_wrapper<PackNode>> InsertNode(int width, int height);

		PackNode() = default;
		PackNode(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h){};
	};

	class TexturePack : public rawrbox::TextureBase {
	private:
		size_t _spriteCount = 0;
		std::unique_ptr<PackNode> _root;

	public:
		uint32_t size;
		explicit TexturePack(uint32_t size = 1024);

		TexturePack(const TexturePack&) = delete;
		TexturePack(TexturePack&&) = delete;
		TexturePack& operator=(const TexturePack&) = delete;
		TexturePack& operator=(TexturePack&&) = delete;
		~TexturePack() override;

		[[nodiscard]] size_t getSpriteCount() const;

		bool canInsertNode(int width, int height);
		PackNode& addSprite(int width, int height, const std::vector<uint8_t>& data);

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
	};
} // namespace rawrbox
