#include <fmt/format.h>
#include <rawrbox/render/texture/render.h>

// Compiled shaders
#include <generated/shaders/render/all.h>

namespace rawrBox {
	uint32_t TextureRender::renderID = 5; // 5 > render textures

	TextureRender::TextureRender(bgfx::ViewId viewId, const rawrBox::Vector2i& size) {
		this->_size = size;
		this->_viewId = viewId;
		this->_renderId = ++TextureRender::renderID;

		// Setup texture target view
		bgfx::setViewName(this->_renderId, fmt::format("RAWR-RENDER-VIEW-{}", this->_renderId).c_str());
		bgfx::setViewRect(this->_renderId, 0, 0, size.x, size.y);
		bgfx::setViewMode(this->_renderId, bgfx::ViewMode::Default);
		bgfx::setViewClear(this->_renderId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x00000000, 1.0f, 0);
		// ------

		this->_renderView = bgfx::createFrameBuffer(size.x, size.y, bgfx::TextureFormat::RGBA8, BGFX_TEXTURE_RT);
		bgfx::setName(this->_renderView, fmt::format("RAWR-RENDER-FRAME-BUFFER-{}", this->_renderView.idx).c_str());

		this->_handle = bgfx::getTexture(this->_renderView, 0);
		bgfx::setName(this->_handle, fmt::format("RAWR-RENDER-TARGET-{}", this->_handle.idx).c_str());
	}

	void TextureRender::startRecord() {
		if (!bgfx::isValid(this->_renderView)) return;

		bgfx::setViewFrameBuffer(this->_renderId, this->_renderView);
		bgfx::touch(this->_renderId);
	}

	void TextureRender::stopRecord() {
		if (!bgfx::isValid(this->_renderView)) return;
		bgfx::touch(0);
	}

	void TextureRender::upload() { throw std::runtime_error("Not required"); }
	const bgfx::ViewId TextureRender::id() { return this->_renderId; }
} // namespace rawrBox
