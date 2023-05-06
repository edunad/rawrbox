#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.hpp>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrbox {
	class Renderer {
	private:
		// Default settings
		uint32_t _clearColor = 0x000000FF;
		bgfx::ViewId _id = 0;
		// -----

		rawrbox::Vector2i _size = {};
		std::unique_ptr<rawrbox::Stencil> _stencil = nullptr;

	public:
		virtual ~Renderer();
		Renderer(bgfx::ViewId id, const rawrbox::Vector2i& size);
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void upload();
		void setClearColor(uint32_t clearColor);
		void resizeView(const rawrbox::Vector2i& size);
		void clear();

		// ------RENDERING
		void swapBuffer() const;

#ifdef RAWRBOX_DEBUG
		void render(bool gizmos = false) const;
#else
		void render() const;
#endif
		// --------------------

		// ------UTILS
		[[nodiscard]] bgfx::ViewId getID() const;
		[[nodiscard]] const rawrbox::Vector2i& getSize() const;
		[[nodiscard]] rawrbox::Stencil& getStencil() const;
		[[nodiscard]] uint32_t getClearColor() const;
		// --------------------
	};
} // namespace rawrbox
