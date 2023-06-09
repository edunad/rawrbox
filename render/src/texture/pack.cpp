#include <rawrbox/render/texture/pack.hpp>

#include <fmt/format.h>

#include <memory>

namespace rawrbox {
	TexturePack::TexturePack(uint32_t _size) : size(_size) {
		this->_root = std::make_unique<PackNode>(0, 0, size, size);
	}

	TexturePack::~TexturePack() { this->_root.reset(); }

	size_t TexturePack::getSpriteCount() const {
		return this->_spriteCount;
	}

	PackNode& TexturePack::addSprite(int width, int height, const std::vector<uint8_t>& data) {
		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TexturePack] Texture not bound");

		auto nodeOpt = this->_root->InsertNode(width, height);
		if (!nodeOpt.has_value()) throw std::runtime_error(fmt::format("[TexturePack] Error: failed to add sprite with size {}, {}", width, height));

		this->_spriteCount++;
		auto& node = (*nodeOpt).get();

		if (!data.empty()) bgfx::updateTexture2D(this->_handle, 0, 0, node.x, node.y, node.width, node.height, bgfx::copy(data.data(), static_cast<uint32_t>(data.size())));
		return node;
	}

	bool TexturePack::canInsertNode(int width, int height) {
		return this->_root->canInsertNode(width, height);
	}

	bool PackNode::canInsertNode(int insertedWidth, int insertedHeight) {
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
			left = std::make_unique<PackNode>(x, y, insertedWidth, height);
			right = std::make_unique<PackNode>(x + insertedWidth, y, remainingWidth, height);
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left = std::make_unique<PackNode>(x, y, width, insertedHeight);
			right = std::make_unique<PackNode>(x, y + insertedHeight, width, remainingHeight);
		}

		return left->canInsertNode(insertedWidth, insertedHeight);
	}

	std::optional<std::reference_wrapper<PackNode>> PackNode::InsertNode(int insertedWidth, int insertedHeight) {
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
			left = std::make_unique<PackNode>(x, y, insertedWidth, height);
			right = std::make_unique<PackNode>(x + insertedWidth, y, remainingWidth, height);
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left = std::make_unique<PackNode>(x, y, width, insertedHeight);
			right = std::make_unique<PackNode>(x, y + insertedHeight, width, remainingHeight);
		}

		return left->InsertNode(insertedWidth, insertedHeight);
	}

	void TexturePack::upload(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_handle)) return; // Already bound
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->size), static_cast<uint16_t>(this->size), false, 0, format, 0 | this->_flags);

		if (!bgfx::isValid(this->_handle)) throw std::runtime_error("[TexturePack] Failed to bind texture");
		bgfx::setName(this->_handle, fmt::format("RAWR-PACK-TEXTURE-{}", this->_handle.idx).c_str());
	}

} // namespace rawrbox
