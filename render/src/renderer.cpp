

#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/window.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	Renderer::Renderer(bgfx::ViewId id, const rawrbox::Vector2i& size) : _id(id), _size(size) {
		if (!rawrbox::BGFX_INITIALIZED) return;

		bgfx::setViewRect(id, 0, 0, size.x, size.y);
		bgfx::setViewMode(id, bgfx::ViewMode::Default);
		bgfx::setViewName(id, fmt::format("RawrBox-RENDERER-{}", id).c_str());

		bgfx::setViewClear(id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);
		// ----
	}

	void Renderer::setClearColor(uint32_t clearColor) {
		this->_clearColor = clearColor;
	}

	void Renderer::resizeView(const rawrbox::Vector2i& size) {
		if (!rawrbox::BGFX_INITIALIZED) return;

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);
	}

	// ------RENDERING
	void Renderer::clear() const {
		if (!rawrbox::BGFX_INITIALIZED) return;

		bgfx::touch(this->_id); // Make sure we draw on the view
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);
	}
	// --------------------

	// ------UTILS
	bgfx::ViewId Renderer::getID() const { return this->_id; }
	const rawrbox::Vector2i& Renderer::getSize() const { return this->_size; }
	uint32_t Renderer::getClearColor() const { return this->_clearColor; }
	// --------------------
} // namespace rawrbox
