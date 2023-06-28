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
		bgfx::TextureHandle _depthHandle = BGFX_INVALID_HANDLE;

		rawrbox::Vector2i _size;

		bgfx::ViewId _prevViewId;
		bgfx::ViewId _renderId;

		bool _depth;

	public:
		explicit TextureRender(const rawrbox::Vector2i& size, bgfx::ViewId id = (rawrbox::RENDERER_VIEW_ID + ++TextureRender::renderID), bool depth = true);

		TextureRender(TextureRender&&) = delete;
		TextureRender& operator=(TextureRender&&) = delete;
		TextureRender(const TextureRender&) = delete;
		TextureRender& operator=(const TextureRender&) = delete;

		~TextureRender() override;

		// ------UTILS
		[[nodiscard]] virtual const bgfx::TextureHandle& getDepth() const;
		[[nodiscard]] virtual const bgfx::FrameBufferHandle& getBuffer() const;
		// ------------

		// ------RENDER
		virtual void startRecord(bool clear = true);
		virtual void stopRecord();

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------

		virtual const bgfx::ViewId id();
	};
} // namespace rawrbox
