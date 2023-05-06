#pragma once

#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

#include <functional>
#include <memory>
#include <optional>

namespace rawrbox {
	struct AtlasNode {
		int x = 0;
		int y = 0;
		int width = 0;
		int height = 0;
		bool empty = true;

		std::unique_ptr<AtlasNode> left = nullptr;
		std::unique_ptr<AtlasNode> right = nullptr;

		bool canInsertNode(int insertedWidth, int insertedHeight);
		std::optional<std::reference_wrapper<AtlasNode>> InsertNode(int width, int height);

		AtlasNode() = default;
		AtlasNode(int _x, int _y, int _w, int _h) : x(_x), y(_y), width(_w), height(_h){};
	};

	class TextureAtlas : public rawrbox::TextureBase {
	private:
		size_t _spriteCount = 0;
		std::unique_ptr<AtlasNode> _root;

	public:
		uint32_t size;
		explicit TextureAtlas(uint32_t size = 1024);

		[[nodiscard]] size_t getSpriteCount() const;

		bool canInsertNode(int width, int height);
		AtlasNode& addSprite(int width, int height, const std::vector<unsigned char>& data);

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
	};
} // namespace rawrbox
