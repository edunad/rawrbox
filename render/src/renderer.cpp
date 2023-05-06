#ifdef RAWRBOX_DEBUG
	#include <rawrbox/debug/gizmos.hpp>
#endif

#include <rawrbox/render/renderer.hpp>
#include <rawrbox/render/static.hpp>

#include <fmt/format.h>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_STENCIL | BGFX_CLEAR_DEPTH)

namespace rawrbox {
	Renderer::~Renderer() {
		this->_stencil = nullptr;

		rawrbox::MISSING_TEXTURE = nullptr;
		rawrbox::MISSING_SPECULAR_TEXTURE = nullptr;
		rawrbox::WHITE_TEXTURE = nullptr;

#ifdef RAWRBOX_DEBUG
		rawrbox::GIZMOS::get().shutdown();
#endif
	}

	Renderer::Renderer(bgfx::ViewId id, const rawrbox::Vector2i& size) : _id(id), _size(size) {
		if (!rawrbox::BGFX_INITIALIZED) return;
		this->_stencil = std::make_unique<rawrbox::Stencil>(id, size);

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewMode(this->_id, bgfx::ViewMode::Sequential);
		bgfx::setViewName(this->_id, fmt::format("RawrBox-RENDERER-{}", this->_id).c_str());

		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);

		rawrbox::MISSING_TEXTURE = std::make_shared<rawrbox::TextureMissing>();
		rawrbox::MISSING_SPECULAR_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::Black);
		rawrbox::WHITE_TEXTURE = std::make_shared<rawrbox::TextureFlat>(rawrbox::Vector2i(2, 2), rawrbox::Colors::White);
	}

	void Renderer::upload() {
		if (!rawrbox::BGFX_INITIALIZED) return;

		rawrbox::MISSING_TEXTURE->upload();
		rawrbox::MISSING_SPECULAR_TEXTURE->upload();
		rawrbox::WHITE_TEXTURE->upload();

		// Debug gizmos ----
#ifdef RAWRBOX_DEBUG
		rawrbox::GIZMOS::get().upload();
#endif
		// -----

		if (this->_stencil == nullptr) throw std::runtime_error("[RawrBox-Renderer] Failed to upload, stencil is not initialized!");
		this->_stencil->upload();
	}

	void Renderer::setClearColor(uint32_t clearColor) {
		this->_clearColor = clearColor;
	}

	void Renderer::resizeView(const rawrbox::Vector2i& size) {
		if (!rawrbox::BGFX_INITIALIZED) return;

		bgfx::setViewRect(this->_id, 0, 0, size.x, size.y);
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);

		if (this->_stencil != nullptr) this->_stencil->resize(size);
	}

	// ------RENDERING
	void Renderer::swapBuffer() const {
		if (!rawrbox::BGFX_INITIALIZED) return;

		bgfx::touch(this->_id); // Make sure we draw on the view
		bgfx::setViewClear(this->_id, BGFX_DEFAULT_CLEAR, this->_clearColor, 1.0F, 0);
	}

#ifdef RAWRBOX_DEBUG
	void Renderer::render(bool gizmos) const {
		if (!rawrbox::BGFX_INITIALIZED) return;

		if (gizmos) rawrbox::GIZMOS::get().draw();
		bgfx::frame();
	}
#else
	void Renderer::render() const {
		if (!rawrbox::BGFX_INITIALIZED) return;
		bgfx::frame();
	}
#endif
	// --------------------

	// ------UTILS
	bgfx::ViewId Renderer::getID() const {
		return this->_id;
	}

	const rawrbox::Vector2i& Renderer::getSize() const {
		return this->_size;
	}

	rawrbox::Stencil& Renderer::getStencil() const {
		return *this->_stencil;
	}

	uint32_t Renderer::getClearColor() const {
		return this->_clearColor;
	}
	// --------------------
} // namespace rawrbox
