
#include <rawrbox/render/texture/render.hpp>

#include <fmt/format.h>

#include <array>

namespace rawrbox {
	uint32_t TextureRender::renderID = 0;
	TextureRender::TextureRender(const rawrbox::Vector2i& size, bool depth, bgfx::ViewId id) : _size(size), _prevViewId(rawrbox::CURRENT_VIEW_ID), _renderId(id), _depth(depth) {
		// Setup texture target view
		bgfx::setViewName(this->_renderId, fmt::format("RAWR-RENDER-VIEW-{}", this->_renderId).c_str());
		bgfx::setViewRect(this->_renderId, 0, 0, this->_size.x, this->_size.y);
		bgfx::setViewMode(this->_renderId, bgfx::ViewMode::Default);
		//   ------
	}

	TextureRender::~TextureRender() {
		RAWRBOX_DESTROY(this->_depthHandle);
		RAWRBOX_DESTROY(this->_renderView);
	}

	// ------ UTILS
	const bgfx::TextureHandle& TextureRender::getDepth() const { return this->_depthHandle; }
	const bgfx::FrameBufferHandle& TextureRender::getBuffer() const { return this->_renderView; }
	// ------------

	// ------ RENDER
	void TextureRender::startRecord(bool clear) {
		if (!bgfx::isValid(this->_renderView)) return;

		this->_prevViewId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = this->_renderId;

		bgfx::setViewFrameBuffer(this->_renderId, this->_renderView);
		bgfx::setViewClear(this->_renderId, clear ? BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL : BGFX_CLEAR_NONE, 0x00000000, 1.F, 0);
		bgfx::touch(this->_renderId);
	}

	void TextureRender::stopRecord() {
		if (!bgfx::isValid(this->_renderView)) return;

		rawrbox::CURRENT_VIEW_ID = this->_prevViewId;
		bgfx::touch(this->_prevViewId);
	}

	void TextureRender::upload(bgfx::TextureFormat::Enum format) {
		this->_handle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 1, format, BGFX_TEXTURE_RT | this->_flags);
		bgfx::setName(this->_handle, fmt::format("RAWR-RENDER-TARGET-{}", this->_handle.idx).c_str());

		if (_depth) {
			bgfx::TextureFormat::Enum depthFormat =
			    bgfx::isTextureValid(0, false, 1, bgfx::TextureFormat::D32, BGFX_TEXTURE_RT_WRITE_ONLY | this->_flags)
				? bgfx::TextureFormat::D32
				: bgfx::TextureFormat::D16;

			this->_depthHandle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | this->_flags);
			bgfx::setName(this->_depthHandle, fmt::format("RAWR-RENDER-TARGET-DEPTH-{}", this->_depthHandle.idx).c_str());

			std::array<bgfx::TextureHandle, 2> texHandles = {this->_handle, this->_depthHandle};
			this->_renderView = bgfx::createFrameBuffer(2, texHandles.data());
		} else {
			this->_renderView = bgfx::createFrameBuffer(1, &this->_handle);
		}
	} // ------------

	const bgfx::ViewId TextureRender::id() { return this->_renderId; }
} // namespace rawrbox
