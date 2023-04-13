#pragma once
#include <rawrbox/render/texture/atlas.h>
#include <fmt/format.h>

namespace rawrBox {
	TextureAtlas::TextureAtlas(uint32_t _size, bgfx::TextureFormat::Enum _format) {
		this->size = _size;

		this->_format = _format;
		this->_root = std::make_unique<AtlasNode>(0, 0, size, size, true, nullptr, nullptr);
	}

	size_t TextureAtlas::getSpriteCount() const {
		return this->_spriteCount;
	}

	AtlasNode& TextureAtlas::addSprite(int width, int height, const std::vector<unsigned char>& data) {
		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureAtlas] Texture not bound");

		auto nodeOpt = this->_root->InsertNode(width, height);
		if (!nodeOpt.has_value()) throw std::runtime_error(fmt::format("[TextureAtlas] Error: failed to add sprite with size {}, {}", width, height));

		this->_spriteCount++;
		auto& node = (*nodeOpt).get();

		if(!data.empty()) bgfx::updateTexture2D(this->_handle, 0, 0, node.x, node.y, node.width, node.height, bgfx::copy(data.data(), static_cast<uint32_t>(data.size())));
		return node;
	}

	bool TextureAtlas::canInsertNode(int width, int height) {
		return this->_root->canInsertNode(width, height);
	}

	bool AtlasNode::canInsertNode(int insertedWidth, int insertedHeight) {
		if (!empty) return false;
		if (left && right) {
			if (left->canInsertNode(insertedWidth, insertedHeight)) return true;
			if (right->canInsertNode(insertedWidth, insertedHeight)) return true;

			return false;
		}

		if (insertedWidth > width || insertedHeight > height) return false;
		if (width == insertedWidth && height == insertedHeight) return true;

		// if all of the above didn't return, the current leaf is large enough,
		// with some space to spare, so we split up the current node so we have
		// one prefectly fitted node and some spare nodes
		int remainingWidth = width - insertedWidth;
		int remainingHeight = height - insertedHeight;

		bool isRemainderWiderThanHigh = remainingWidth > remainingHeight;

		if (isRemainderWiderThanHigh) { // if wider than high, split verticallly
			left  = std::unique_ptr<AtlasNode>(new AtlasNode{x,                 y, insertedWidth,  height, true, nullptr, nullptr});
			right = std::unique_ptr<AtlasNode>(new AtlasNode{x + insertedWidth, y, remainingWidth, height, true, nullptr, nullptr});
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left  = std::unique_ptr<AtlasNode>(new AtlasNode{x, y,                  width, insertedHeight,  true, nullptr, nullptr});
			right = std::unique_ptr<AtlasNode>(new AtlasNode{x, y + insertedHeight, width, remainingHeight, true, nullptr, nullptr});
		}

		return left->canInsertNode(insertedWidth, insertedHeight);
	}

	std::optional<std::reference_wrapper<AtlasNode>> AtlasNode::InsertNode(int insertedWidth, int insertedHeight) {
		if (!empty) return std::nullopt;

		if (left && right) {
			// both children exist, which means this node is full, try left then right
			if (auto node = left->InsertNode(insertedWidth, insertedHeight); node != std::nullopt) {
				return node;
			}

			if (auto node = right->InsertNode(insertedWidth, insertedHeight); node != std::nullopt) {
				return node;
			}

			return std::nullopt;
		}

		if (insertedWidth > width || insertedHeight > height) {
			// to be insterted node is too big, can't fit here
			return std::nullopt;
		}

		if (width == insertedWidth && height == insertedHeight) {
			// fits perfectly
			empty = false;
			return *this;
		}

		// if all of the above didn't return, the current leaf is large enough,
		// with some space to spare, so we split up the current node so we have
		// one prefectly fitted node and some spare nodes
		int remainingWidth = width - insertedWidth;
		int remainingHeight = height - insertedHeight;

		bool isRemainderWiderThanHigh = remainingWidth > remainingHeight;

		if (isRemainderWiderThanHigh) { // if wider than high, split verticallly
			left  = std::unique_ptr<AtlasNode>(new AtlasNode{x,                 y, insertedWidth,  height, true, nullptr, nullptr});
			right = std::unique_ptr<AtlasNode>(new AtlasNode{x + insertedWidth, y, remainingWidth, height, true, nullptr, nullptr});
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left  = std::unique_ptr<AtlasNode>(new AtlasNode{x, y,                  width, insertedHeight,  true, nullptr, nullptr});
			right = std::unique_ptr<AtlasNode>(new AtlasNode{x, y + insertedHeight, width, remainingHeight, true, nullptr, nullptr});
		}

		return left->InsertNode(insertedWidth, insertedHeight);
	}

	void TextureAtlas::upload() {
		if(bgfx::isValid(this->_handle)) return; // Already bound
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->size), static_cast<uint16_t>(this->size), false, 0, this->_format);

		if(!bgfx::isValid(this->_handle)) throw std::runtime_error("[TextureAtlas] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-ATLAS-TEXTURE-{}", this->_handle.idx).c_str());
	}

}
