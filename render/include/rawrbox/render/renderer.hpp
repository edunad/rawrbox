#pragma once

#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

#include <memory>

namespace rawrbox {
	class Window;
	class Renderer {
	private:
		// Default settings
		uint32_t _clearColor = 0x000000FF;
		bgfx::ViewId _id = 0;
		// -----

		rawrbox::Vector2i _size = {};

	public:
		virtual ~Renderer() = default;

		Renderer(bgfx::ViewId id, const rawrbox::Vector2i& size);
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;
		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		virtual void setClearColor(uint32_t clearColor);
		virtual void resizeView(const rawrbox::Vector2i& size);

		// ------RENDERING
		virtual void clear() const;
		// --------------------

		// ------UTILS
		[[nodiscard]] virtual bgfx::ViewId getID() const;
		[[nodiscard]] virtual const rawrbox::Vector2i& getSize() const;
		[[nodiscard]] virtual uint32_t getClearColor() const;
		// --------------------
	};
} // namespace rawrbox
