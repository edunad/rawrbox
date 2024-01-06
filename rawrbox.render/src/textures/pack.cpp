
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/pack.hpp>

#include <Align.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TexturePack::TexturePack(uint16_t size) {
		this->_size = {size, size};
		this->_root = std::make_unique<PackNode>(0, 0, size, size);
		this->_name = "RawrBox::Texture::Pack";
	}

	TexturePack::~TexturePack() { this->_root.reset(); }

	size_t TexturePack::getSpriteCount() const {
		return this->_spriteCount;
	}

	rawrbox::PackNode& TexturePack::addSprite(uint16_t width, uint16_t height, const std::vector<uint8_t>& data) {
		if (this->_tex == nullptr) throw std::runtime_error("[RawrBox-TexturePack] Texture not bound");

		auto nodeOpt = this->_root->InsertNode(width, height);
		if (!nodeOpt.has_value()) throw std::runtime_error(fmt::format("[TexturePack] Error: failed to add sprite with size {}, {}", width, height));

		this->_spriteCount++;
		auto& node = (*nodeOpt).get();

		if (!data.empty()) {
			Diligent::Box UpdateBox;
			UpdateBox.MinX = node.x;
			UpdateBox.MinY = node.y;
			UpdateBox.MaxX = node.x + node.width;
			UpdateBox.MaxY = node.y + node.height;

			Diligent::TextureSubResData SubresData;
			SubresData.Stride = static_cast<uint64_t>(node.width * this->_channels);
			SubresData.pData = data.data();

			rawrbox::RENDERER->context()->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		}

		return node;
	}

	bool TexturePack::canInsertNode(uint16_t width, uint16_t height) {
		return this->_root->canInsertNode(width, height);
	}

	bool PackNode::canInsertNode(uint16_t insertedWidth, uint16_t insertedHeight) {
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
			left = std::make_unique<rawrbox::PackNode>(x, y, insertedWidth, height);
			right = std::make_unique<rawrbox::PackNode>(x + insertedWidth, y, remainingWidth, height);
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left = std::make_unique<rawrbox::PackNode>(x, y, width, insertedHeight);
			right = std::make_unique<rawrbox::PackNode>(x, y + insertedHeight, width, remainingHeight);
		}

		return left->canInsertNode(insertedWidth, insertedHeight);
	}

	std::optional<std::reference_wrapper<rawrbox::PackNode>> PackNode::InsertNode(uint16_t insertedWidth, uint16_t insertedHeight) {
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
			left = std::make_unique<rawrbox::PackNode>(x, y, insertedWidth, height);
			right = std::make_unique<rawrbox::PackNode>(x + insertedWidth, y, remainingWidth, height);
		} else { // That'd make the remainder higher than it's wide, split horizontally
			left = std::make_unique<rawrbox::PackNode>(x, y, width, insertedHeight);
			right = std::make_unique<rawrbox::PackNode>(x, y + insertedHeight, width, remainingHeight);
		}

		return left->InsertNode(insertedWidth, insertedHeight);
	}

	void TexturePack::upload(Diligent::TEXTURE_FORMAT format, bool /*dynamic*/) {
		if (this->_failedToLoad || this->_handle != nullptr) return; // Failed texture is already bound, so skip it
		rawrbox::TextureBase::upload(format, true);
	}

} // namespace rawrbox
