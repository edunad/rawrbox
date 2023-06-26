#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <fmt/format.h>

#include <array>

namespace rawrbox {
	uint32_t TextureRender::renderID = 0;
	TextureRender::TextureRender(const rawrbox::Vector2i& size) : _size(size), _prevViewId(rawrbox::CURRENT_VIEW_ID), _renderId(rawrbox::RENDERER_VIEW_ID + ++TextureRender::renderID) {
		// Setup texture target view
		bgfx::setViewName(this->_renderId, fmt::format("RAWR-RENDER-VIEW-{}", this->_renderId).c_str());
		bgfx::setViewRect(this->_renderId, 0, 0, this->_size.x, this->_size.y);
		bgfx::setViewMode(this->_renderId, bgfx::ViewMode::Default);
		bgfx::setViewClear(this->_renderId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL);
		//   ------
	}

	TextureRender::~TextureRender() {
		RAWRBOX_DESTROY(this->_depthHandle);
		RAWRBOX_DESTROY(this->_renderHandle);
	}

	void TextureRender::startRecord(bool clear) {
		if (!bgfx::isValid(this->_renderView)) return;

		this->_prevViewId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = this->_renderId;

		bgfx::setViewFrameBuffer(this->_renderId, this->_renderView);
		bgfx::setViewRect(this->_renderId, 0, 0, this->_size.x, this->_size.y);
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

		this->_depthHandle = bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 1, bgfx::TextureFormat::D24S8, BGFX_TEXTURE_RT | BGFX_TEXTURE_RT_WRITE_ONLY);
		bgfx::setName(this->_depthHandle, fmt::format("RAWR-RENDER-TARGET-DEPTH-{}", this->_depthHandle.idx).c_str());

		std::array<bgfx::TextureHandle, 2> texHandles = {this->_handle, this->_depthHandle};
		this->_renderView = bgfx::createFrameBuffer(2, texHandles.data());
	}

	const bgfx::ViewId TextureRender::id() { return this->_renderId; }
} // namespace rawrbox
