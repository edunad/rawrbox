#include <rawrbox/math/utils/color.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/streaming.hpp>
#include <rawrbox/render/textures/utils/utils.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <cstring>

namespace rawrbox {
	TextureStreaming::TextureStreaming(const rawrbox::Vector2u& size) {
		this->setName("STREAMING");

		this->_data.size = size;
		this->_data.channels = 4U;
		this->_data.createFrame();
	}

	// PROTECTED ---
	void TextureStreaming::pending() {
		if (this->_pendingUpdate) return;
		this->_pendingUpdate = true;

		rawrbox::BindlessManager::registerUpdateTexture(*this);
	}
	// -------------

	// UTILS ---
	void TextureStreaming::clear() {
		this->_hasData = false;
		this->_pendingUpdate = false;

		this->_dirty.clear();
		this->_dirtyAll = false;
	}

	void TextureStreaming::setImage(const rawrbox::ImageData& data) {
		if (data.frames.empty()) RAWRBOX_CRITICAL("Cannot set empty data");

		auto frame = data.pixels();
		if (frame.empty()) RAWRBOX_CRITICAL("Cannot set empty pixels");

		if (data.channels != this->_data.channels) frame = rawrbox::ColorUtils::setChannels(data.channels, this->_data.channels, data.size.x, data.size.y, frame);
		frame = rawrbox::TextureUtils::resize(data.size, frame, this->_data.size, this->_data.channels);

		std::memcpy(this->_data.pixels().data(), frame.data(), frame.size());
		this->markDirty();
	}

	bool TextureStreaming::hasData() const {
		return this->_hasData;
	}

	std::vector<uint8_t>& TextureStreaming::pixels() {
		return this->_data.pixels();
	}

	void TextureStreaming::markDirty(std::optional<rawrbox::AABBu> rect) {
		if (this->_data.empty()) RAWRBOX_CRITICAL("Cannot update empty data");

		const auto& size = this->_data.size;
		this->_hasData = true;

		if (!rect.has_value()) {
			this->_dirty.clear();
			this->_dirtyAll = true;
			this->pending();
			return;
		}

		rawrbox::AABBu& area = rect.value();

		area.pos = {std::min(area.pos.x, size.x), std::min(area.pos.y, size.y)};
		area.size = {std::min(area.size.x, size.x - area.pos.x), std::min(area.size.y, size.y - area.pos.y)};

		if (area.size.x == 0U || area.size.y == 0U) return;
		// ------

		if (!this->_dirtyAll) {
			bool merged = true;
			while (merged) {
				merged = false;

				for (auto it = this->_dirty.begin(); it != this->_dirty.end(); ++it) {
					if (!area.contains(*it)) continue;

					area.combine(*it);
					this->_dirty.erase(it);

					merged = true;
					break;
				}
			}

			this->_dirty.push_back(area);

			if (this->_dirty.size() > this->MAX_DIRTY) {
				auto all = this->_dirty.front();
				for (const auto& dirty : this->_dirty) {
					all.combine(dirty);
				}

				this->_dirty = {all};
			}

			if (this->_dirty.size() == 1U && this->_dirty.front().pos == rawrbox::Vector2u{} && this->_dirty.front().size == size) {
				this->_dirty.clear();
				this->_dirtyAll = true;
			}
		}

		this->pending();
	}
	// --------

	void TextureStreaming::update() {
		if (!this->_pendingUpdate || this->_tex == nullptr) return;
		if (this->_data.empty()) RAWRBOX_CRITICAL("Cannot update empty data");

		auto* context = rawrbox::RENDERER->context();

		const auto& size = this->_data.size;
		const auto* pixels = this->_data.pixels().data();
		const size_t stride = static_cast<size_t>(size.x) * this->_data.channels;

		const auto upload = [&](const rawrbox::AABBu& rect) {
			const Diligent::Box box = {rect.left(), rect.right(), rect.top(), rect.bottom()};

			Diligent::TextureSubResData SubresData;
			SubresData.Stride = stride;
			SubresData.pData = pixels + (static_cast<size_t>(rect.pos.y) * stride) + (static_cast<size_t>(rect.pos.x) * this->_data.channels);

			context->UpdateTexture(this->_tex, 0, 0, box, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		};

		// BARRIER ----
		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

		if (this->_dirtyAll || this->_dirty.empty()) {
			upload({0U, 0U, size.x, size.y});
		} else {
			for (const auto& rect : this->_dirty) {
				upload(rect);
			}
		}

		rawrbox::BarrierUtils::barrier({{this->_tex, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		//  ------------

		this->_dirty.clear();
		this->_dirtyAll = false;
		this->_pendingUpdate = false;
	}

	bool TextureStreaming::requiresUpdate() const {
		return this->_pendingUpdate;
	}

	uint32_t TextureStreaming::getTextureID() const {
		if (!this->hasData()) return rawrbox::MISSING_TEXTURE->getTextureID();
		return rawrbox::TextureBase::getTextureID();
	}

	void TextureStreaming::upload(Diligent::TEXTURE_FORMAT format, uint32_t /*flags*/) {
		rawrbox::TextureBase::upload(format, rawrbox::TextureFlags::DYNAMIC);
	}
} // namespace rawrbox