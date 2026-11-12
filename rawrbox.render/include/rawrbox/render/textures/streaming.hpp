#pragma once

#include <rawrbox/math/aabb.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <optional>
#include <vector>

namespace rawrbox {
	class TextureStreaming : public rawrbox::TextureBase {
	protected:
		static constexpr size_t MAX_DIRTY = 8;

		bool _pendingUpdate = false;
		bool _hasData = false;

		std::vector<rawrbox::AABBu> _dirty = {};
		bool _dirtyAll = true;

		virtual void pending();

	public:
		TextureStreaming(const rawrbox::Vector2u& size);

		// UTILS ---
		void clear();
		void setImage(const rawrbox::ImageData& data);
		[[nodiscard]] bool hasData() const;

		[[nodiscard]] std::vector<uint8_t>& pixels();
		void markDirty(std::optional<rawrbox::AABBu> rect = std::nullopt);
		// --------

		void update() override;
		[[nodiscard]] bool requiresUpdate() const override;

		[[nodiscard]] uint32_t getTextureID() const override;

		void upload(Diligent::TEXTURE_FORMAT format = Diligent::TEXTURE_FORMAT::TEX_FORMAT_UNKNOWN, uint32_t flags = rawrbox::TextureFlags::NONE) override;
	};
} // namespace rawrbox
