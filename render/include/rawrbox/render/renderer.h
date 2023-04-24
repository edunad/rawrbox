#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/stencil.h>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrBox {
	class Renderer {
	private:
		// Default settings
		uint32_t _clearColor = 0x000000FF;
		bgfx::ViewId _id = 0;
		// -----

		rawrBox::Vector2i _size = {};
		std::unique_ptr<rawrBox::Stencil> _stencil = nullptr;

	public:
		~Renderer();
		Renderer(bgfx::ViewId id, const rawrBox::Vector2i& size);
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void upload();
		void setClearColor(uint32_t clearColor);
		void resizeView(const rawrBox::Vector2i& size);
		void clear();

		// ------RENDERING
		void swapBuffer() const;
		void render() const;
		// --------------------

		// ------UTILS
		[[nodiscard]] bgfx::ViewId getID() const;
		[[nodiscard]] const rawrBox::Vector2i& getSize() const;
		[[nodiscard]] rawrBox::Stencil& getStencil() const;
		// --------------------
	};
} // namespace rawrBox
