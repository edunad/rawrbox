#pragma once

#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

#include <functional>
#include <memory>
#include <optional>

namespace rawrBox {
	struct AtlasNode {
		int x;
		int y;
		int width;
		int height;
		bool empty;

		std::unique_ptr<AtlasNode> left;
		std::unique_ptr<AtlasNode> right;

		bool canInsertNode(int insertedWidth, int insertedHeight);
		std::optional<std::reference_wrapper<AtlasNode>> InsertNode(int width, int height);
	};

	class TextureAtlas : public rawrBox::TextureBase {
	private:
		size_t _spriteCount = 0;
		std::unique_ptr<AtlasNode> _root;

	public:
		uint32_t size;
		TextureAtlas(uint32_t size = 1024);

		size_t getSpriteCount() const;

		bool canInsertNode(int width, int height);
		AtlasNode& addSprite(int width, int height, const std::vector<unsigned char>& data);

		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
	};
} // namespace rawrBox
