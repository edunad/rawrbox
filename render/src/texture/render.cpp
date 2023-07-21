
#include <rawrbox/render/texture/render.hpp>

#include <fmt/format.h>

#include <array>

namespace rawrbox {
	uint32_t TextureRender::renderID = 0;
	TextureRender::TextureRender(const rawrbox::Vector2i& size, bgfx::ViewId id) : _size(size), _prevViewId(rawrbox::CURRENT_VIEW_ID), _renderId(id) {
		// Setup texture target view
		bgfx::setViewName(this->_renderId, fmt::format("RAWR-RENDER-VIEW-{}", this->_renderId).c_str());
		bgfx::setViewRect(this->_renderId, 0, 0, this->_size.x, this->_size.y);
		bgfx::setViewMode(this->_renderId, bgfx::ViewMode::Default);
		//   ------
	}

	TextureRender::~TextureRender() {
		RAWRBOX_DESTROY(this->_renderView);
	}

	// ------ UTILS
	void TextureRender::addTexture(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_renderView)) throw std::runtime_error("[RAWRBOX-RenderTarget] Texture already uploaded, cannot add extra textures");
		this->_textureHandles.push_back(bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 1, format, BGFX_TEXTURE_RT | this->_flags));
	}

	bgfx::TextureHandle TextureRender::getTexture(uint8_t i) const {
		if (i < 0 || i >= this->_textureHandles.size()) return BGFX_INVALID_HANDLE;
		return this->_textureHandles[i];
	}

	const bgfx::TextureHandle TextureRender::getDepth() const { return this->_textureHandles.back(); }
	const bgfx::FrameBufferHandle& TextureRender::getBuffer() const { return this->_renderView; }
	// ------------

	// ------ RENDER
	void TextureRender::startRecord(bool clear) {
		if (!bgfx::isValid(this->_renderView)) return;

		this->_prevViewId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = this->_renderId;

		bgfx::setViewFrameBuffer(this->_renderId, this->_renderView);
		bgfx::setViewClear(this->_renderId, clear ? BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH | BGFX_CLEAR_STENCIL : BGFX_CLEAR_NONE, 0x00000000, 1.F);
		bgfx::touch(this->_renderId);
	}

	void TextureRender::stopRecord() {
		if (!bgfx::isValid(this->_renderView)) return;

		rawrbox::CURRENT_VIEW_ID = this->_prevViewId;
		bgfx::touch(this->_prevViewId);
	}

	void TextureRender::upload(bgfx::TextureFormat::Enum format) {
		if (bgfx::isValid(this->_renderView)) throw std::runtime_error("[RAWRBOX-RenderTarget] Already uploaded");

		this->_textureHandles.insert(this->_textureHandles.begin(), bgfx::createTexture2D(static_cast<uint16_t>(this->_size.x), static_cast<uint16_t>(this->_size.y), false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT | this->_flags)); // COLOR
		this->addTexture(bgfx::TextureFormat::D24);                                                                                                                                                                                           // DEPTH

		this->_renderView = bgfx::createFrameBuffer(this->_textureHandles.size(), this->_textureHandles.data(), false);
		this->_handle = this->_textureHandles.front();
	} // ------------

	const bgfx::ViewId TextureRender::id() { return this->_renderId; }
} // namespace rawrbox
