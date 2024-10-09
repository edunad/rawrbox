
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/pack.hpp>

#include <Align.hpp>

#include <fmt/format.h>

namespace rawrbox {
	TexturePack::TexturePack(uint32_t size) {
		this->_data.size = {size, size};
		this->_data.channels = 4;
		this->_data.createFrame(); // Create empty frame to be filled later

		this->_root = std::make_unique<rawrbox::PackNode>(0U, 0U, size, size);
		this->_name = "RawrBox::Texture::Pack";
	}

	TexturePack::~TexturePack() { this->_root.reset(); }

	size_t TexturePack::getSpriteCount() const {
		return this->_spriteCount;
	}

	rawrbox::PackNode& TexturePack::addSprite(uint32_t width, uint32_t height, const std::vector<uint8_t>& data) {
		if (this->_tex == nullptr) CRITICAL_RAWRBOX("Texture not bound");

		auto nodeOpt = this->_root->InsertNode(width, height);
		if (!nodeOpt.has_value()) CRITICAL_RAWRBOX("Failed to add sprite with size {}, {}", width, height);

		auto& node = (*nodeOpt).get();
		node.data = data; // Set the data of that node

		if (!data.empty()) {
			Diligent::Box UpdateBox;
			UpdateBox.MinX = node.x;
			UpdateBox.MinY = node.y;
			UpdateBox.MaxX = node.x + node.width;
			UpdateBox.MaxY = node.y + node.height;

			for (size_t y = 0; y < node.height; y++) {
				const auto stride = node.width * this->_data.channels;
				const auto* start = data.data() + y * stride;
				const auto* last = start + stride;

				auto startPointY = UpdateBox.MinY + y;
				auto startPointX = UpdateBox.MinX;

				auto* dest = this->_data.pixels().data() + (startPointY * this->_data.size.x + startPointX) * this->_data.channels;
				std::copy(start, last, dest);
			}

			if (!this->_pendingUpdate) {
				this->_pendingUpdate = true;
				rawrbox::BindlessManager::registerUpdateTexture(*this);
			}
		}

		this->_spriteCount++;
		return node;
	}

	bool TexturePack::canInsertNode(uint32_t width, uint32_t height) {
		return this->_root->canInsertNode(width, height);
	}

	bool PackNode::canInsertNode(uint32_t insertedWidth, uint32_t insertedHeight) {
		if (!data.empty()) return false;

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
		int remainingWidth = std::max<int>(0, static_cast<int>(width) - static_cast<int>(insertedWidth));
		int remainingHeight = std::max<int>(0, static_cast<int>(height) - static_cast<int>(insertedHeight));

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

	std::optional<std::reference_wrapper<rawrbox::PackNode>> PackNode::InsertNode(uint32_t insertedWidth, uint32_t insertedHeight) {
		if (!data.empty()) return std::nullopt;

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

	void TexturePack::update() {
		if (!this->_pendingUpdate) return;
		auto* context = rawrbox::RENDERER->context();

		Diligent::Box UpdateBox;
		UpdateBox.MinX = 0;
		UpdateBox.MinY = 0;
		UpdateBox.MaxX = this->_data.size.x;
		UpdateBox.MaxY = this->_data.size.y;

		Diligent::TextureSubResData SubresData;
		SubresData.Stride = this->_data.size.x * this->_data.channels;
		SubresData.pData = this->_data.pixels().data();

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->UpdateTexture(this->_tex, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ------------

		this->_pendingUpdate = false;
	}

	[[nodiscard]] bool TexturePack::requiresUpdate() const { return this->_pendingUpdate; };
} // namespace rawrbox
