#pragma once
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <functional>
#include <string>

namespace rawrbox {
	class RendererBase {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;

		rawrbox::Vector2i _size = {};

		virtual void frame();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;

		RendererBase() = default;
		RendererBase(const RendererBase&) = delete;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = delete;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init(const rawrbox::Vector2i& size);
		virtual void resize(const rawrbox::Vector2i& size);

		virtual void setWorldRender(std::function<void()> render);
		virtual void setOverlayRender(std::function<void()> render);

		virtual void render();
		virtual void bindRenderUniforms();

		// Utils ----
		virtual bgfx::TextureHandle getDepth();
		virtual bgfx::TextureHandle getColor();
		// ------

		static bool supported();
	};
} // namespace rawrbox
