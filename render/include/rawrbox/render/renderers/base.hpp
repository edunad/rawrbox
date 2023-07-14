#pragma once
#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <functional>
#include <string>

namespace rawrbox {
	class RendererBase {
	protected:
		rawrbox::Vector2i _size = {};
		bgfx::FrameBufferHandle _frameBuffer = BGFX_INVALID_HANDLE; // Used for getting the depth

		virtual void frame();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;

		RendererBase() = default;
		RendererBase(const RendererBase&) = default;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = default;
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
