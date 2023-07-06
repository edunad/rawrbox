#pragma once
#include <rawrbox/math/vector2.hpp>

#include <bgfx/bgfx.h>

#include <array>
#include <functional>
#include <string>

namespace rawrbox {
	class RendererBase {
	protected:
		bgfx::FrameBufferHandle _frameBuffer = BGFX_INVALID_HANDLE;

		rawrbox::Vector2i _size = {};

	public:
		std::function<void()> worldRender = nullptr;

		RendererBase() = default;
		RendererBase(const RendererBase&) = default;
		RendererBase(RendererBase&&) = delete;
		RendererBase& operator=(const RendererBase&) = default;
		RendererBase& operator=(RendererBase&&) = delete;
		virtual ~RendererBase();

		virtual void init(const rawrbox::Vector2i& size);
		virtual void resize(const rawrbox::Vector2i& size);
		virtual void setWorldRender(std::function<void()> render);

		virtual void render();
		virtual void postRender();

		virtual void bindRenderUniforms();

		static bool supported();
	};
} // namespace rawrbox
