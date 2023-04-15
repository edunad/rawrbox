#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>

namespace rawrBox {
	class TextureRender : public TextureBase {
		static uint32_t renderID;

	private:
		bgfx::FrameBufferHandle _renderView = BGFX_INVALID_HANDLE;
		rawrBox::Vector2i _size;

		bgfx::ViewId _viewId;
		bgfx::ViewId _renderId;

	public:
		TextureRender(bgfx::ViewId viewId, const rawrBox::Vector2i& size);

		// ------RENDER
		virtual void startRecord();
		virtual void stopRecord();

		virtual void upload(bgfx::TextureFormat::Enum format = bgfx::TextureFormat::RGBA8) override;
		// --------------------

		virtual const bgfx::ViewId id();
	};
} // namespace rawrBox
