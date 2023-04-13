#pragma once

#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

#include <memory>
#include <optional>
#include <functional>

namespace rawrBox {
	struct Sprite {
		int x;
		int y;
		int width;
		int height;
	};

	struct AtlasNode {
		int x;
		int y;
		int width;
		int height;
		bool empty;

		std::unique_ptr<AtlasNode> left;
		std::unique_ptr<AtlasNode> right;

		std::optional<std::reference_wrapper<AtlasNode>> InsertNode(int width, int height);
	};

	class TextureAtlas: public rawrBox::TextureBase {
	private:
		size_t _spriteCount = 0;
		bgfx::TextureFormat::Enum _format;

		std::unique_ptr<AtlasNode> _root;
	public:
		uint32_t size;
		TextureAtlas(uint32_t size = 1024, bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8);

		size_t getSpriteCount() const;
		AtlasNode& addSprite(int width, int height, const std::vector<unsigned char>& data);

		virtual void upload() override;
	};
}
