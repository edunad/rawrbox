#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureRender : public TextureBase {
		// NOLINTBEGIN{cppcoreguidelines-avoid-non-const-global-variables}
		static uint32_t renderID;
		// NOLINTEND{cppcoreguidelines-avoid-non-const-global-variables}

	private:
		bgfx::FrameBufferHandle _renderView = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _depthHandle = BGFX_INVALID_HANDLE;
		bgfx::TextureHandle _renderHandle = BGFX_INVALID_HANDLE;

		rawrBox::Vector2i _size;

		bgfx::ViewId _viewId;
		bgfx::ViewId _renderId;

	public:
		TextureRender(bgfx::ViewId viewId, const rawrBox::Vector2i& size);

		TextureRender(TextureRender&&) = delete;
		TextureRender& operator=(TextureRender&&) = delete;
		TextureRender(const TextureRender&) = delete;
		TextureRender& operator=(const TextureRender&) = delete;

		~TextureRender() override;

		// ------RENDER
		virtual void startRecord(bool clear = true);
		virtual void stopRecord();

		void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------

		virtual const bgfx::ViewId id();
	};
} // namespace rawrBox
