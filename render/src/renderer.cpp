#include <rawrbox/debug/gizmos.hpp>
#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH)

namespace rawrBox {
	Renderer::~Renderer() {
		this->_stencil = nullptr;

		rawrBox::MISSING_TEXTURE = nullptr;
		rawrBox::MISSING_SPECULAR_TEXTURE = nullptr;
		rawrBox::WHITE_TEXTURE = nullptr;

		rawrBox::GIZMOS::get().shutdown();
	}

	Renderer::Renderer(bgfx::ViewId id, const rawrBox::Vector2i& size) : _id(id), _size(size) {
		if (!rawrBox::BGFX_INITIALIZED) return;
		this->_stencil = std::make_unique<rawrBox::Stencil>(id, size);

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewMode(this->_id, bgfx::ViewMode::Sequential);
		bgfx::setViewName(this->_id, fmt::format("RawrBox-RENDERER-{}", this->_id).c_str());

		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);

		rawrBox::MISSING_TEXTURE = std::make_shared<rawrBox::TextureMissing>();
		rawrBox::MISSING_SPECULAR_TEXTURE = std::make_shared<rawrBox::TextureFlat>(rawrBox::Vector2i(2, 2), rawrBox::Colors::Black);
		rawrBox::WHITE_TEXTURE = std::make_shared<rawrBox::TextureFlat>(rawrBox::Vector2i(2, 2), rawrBox::Colors::White);
	}

	void Renderer::upload() {
		if (!rawrBox::BGFX_INITIALIZED) return;

		rawrBox::MISSING_TEXTURE->upload();
		rawrBox::MISSING_SPECULAR_TEXTURE->upload();
		rawrBox::WHITE_TEXTURE->upload();

		// Debug gizmos ----
		rawrBox::GIZMOS::get().upload();
		// -----

		if (this->_stencil == nullptr) throw std::runtime_error("[RawrBox-Renderer] Failed to upload, stencil is not initialized!");
		this->_stencil->upload();
	}

	void Renderer::setClearColor(uint32_t clearColor) {
		this->_clearColor = clearColor;
	}

	void Renderer::resizeView(const rawrBox::Vector2i& size) {
		if (!rawrBox::BGFX_INITIALIZED) return;

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);

		if (this->_stencil != nullptr) this->_stencil->resize(size);
	}

	// ------RENDERING
	void Renderer::swapBuffer() const {
		if (!rawrBox::BGFX_INITIALIZED) return;

		bgfx::touch(this->_id); // Make sure we draw on the view
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);
	}

	void Renderer::render(bool gizmos) const {
		if (!rawrBox::BGFX_INITIALIZED) return;

		if (gizmos) rawrBox::GIZMOS::get().draw();
		bgfx::frame();
	}
	// --------------------

	// ------UTILS
	bgfx::ViewId Renderer::getID() const {
		return this->_id;
	}

	const rawrBox::Vector2i& Renderer::getSize() const {
		return this->_size;
	}

	rawrBox::Stencil& Renderer::getStencil() const {
		return *this->_stencil;
	}

	uint32_t Renderer::getClearColor() const {
		return this->_clearColor;
	}
	// --------------------
} // namespace rawrBox
