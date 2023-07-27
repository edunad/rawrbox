#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrbox {
	class TextureRender : public TextureBase {
		static uint32_t renderID;

	private:
		bgfx::FrameBufferHandle _renderView = BGFX_INVALID_HANDLE;
		std::vector<bgfx::TextureHandle> _textureHandles = {};

		rawrbox::Vector2i _size;

		bgfx::ViewId _prevViewId;
		bgfx::ViewId _renderId;

	public:
		explicit TextureRender(const rawrbox::Vector2i& size, bgfx::ViewId id = (rawrbox::RENDERER_VIEW_ID + ++TextureRender::renderID));

		TextureRender(TextureRender&&) = delete;
		TextureRender& operator=(TextureRender&&) = delete;
		TextureRender(const TextureRender&) = delete;
		TextureRender& operator=(const TextureRender&) = delete;

		~TextureRender() override;

		// ------UTILS
		virtual void addTexture(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::BGRA8, uint64_t flags = BGFX_TEXTURE_RT);

		[[nodiscard]] virtual const bgfx::TextureHandle getDepth() const;
		[[nodiscard]] virtual bgfx::TextureHandle getTexture(uint8_t i) const;
		[[nodiscard]] virtual const bgfx::FrameBufferHandle& getBuffer() const;
		// ------------

		// ------RENDER
		virtual void startRecord(bool clear = true);
		virtual void stopRecord();

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::BGRA8) override;
		// --------------------

		virtual bgfx::ViewId id();
	};
} // namespace rawrbox
