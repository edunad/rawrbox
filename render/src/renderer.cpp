#include <rawrbox/render/renderer.h>

#include <fmt/format.h>

namespace rawrBox {
	Renderer::~Renderer() {
		this->_stencil = nullptr;
	}

	Renderer::Renderer(bgfx::ViewId id, const rawrBox::Vector2i& size) {
		this->_id = id;
		this->_size = size;
		this->_stencil = std::make_unique<rawrBox::Stencil>(id, size);

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewMode(this->_id, bgfx::ViewMode::Sequential);
		bgfx::setViewName(this->_id, fmt::format("RawrBox-RENDERER-{}", this->_id).c_str());
		bgfx::setViewClear(this->_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH, this->_clearColor, 1.0f, 0);
	}

	void Renderer::upload() {
		if (this->_stencil == nullptr) throw std::runtime_error("[RawrBox-Renderer] Failed to upload, stencil is not initialized!");
		this->_stencil->upload();
	}

	void Renderer::setClearColor(uint32_t clearColor) {
		this->_clearColor = clearColor;
	}

	void Renderer::resizeView(const rawrBox::Vector2i& size) {
		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewClear(this->_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH, this->_clearColor, 1.0f, 0);

		if (this->_stencil != nullptr) this->_stencil->resize(size);
	}

	// ------RENDERING
	void Renderer::swapBuffer() const {
		bgfx::touch(this->_id); // Make sure we draw on the view
		bgfx::setViewClear(this->_id, BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH, this->_clearColor, 1.0f, 0);
	}

	void Renderer::render() const {
		bgfx::frame();
	}
	// --------------------

	// ------UTILS
	bgfx::ViewId Renderer::getID() const {
		return this->_id;
	}

	rawrBox::Vector2i& Renderer::getSize() {
		return this->_size;
	}

	rawrBox::Stencil& Renderer::getStencil() const {
		return *this->_stencil;
	}
	// --------------------
} // namespace rawrBox
