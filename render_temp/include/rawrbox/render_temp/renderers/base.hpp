#pragma once
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render_temp/texture/render.hpp>

#include <bgfx/bgfx.h>

#include <functional>

namespace rawrbox {

	class RendererBase {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		std::unique_ptr<rawrbox::TextureRender> _decals = nullptr;
		bgfx::TextureHandle _GPUBlitTex = BGFX_INVALID_HANDLE;

		rawrbox::Vector2i _size = {};

		// GPU PICKING ----
		uint32_t _gpuReadFrame = 0;
		std::array<uint8_t, rawrbox::GPU_PICK_SAMPLE_SIZE> _gpuPixelData = {};
		std::vector<std::function<void(uint32_t)>> _gpuPickCallbacks = {};
		// ----------------

		virtual void frame();
		virtual void gpuCheck();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;
		std::function<void()> postRender = nullptr;

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
		virtual void overridePostWorld(std::function<void()> post);

		virtual void render();
		virtual void finalRender();
		virtual void bindRenderUniforms();

		// Utils ----
		[[nodiscard]] virtual const bgfx::TextureHandle getDepth() const;
		[[nodiscard]] virtual const bgfx::TextureHandle getColor() const;
		[[nodiscard]] virtual const bgfx::TextureHandle getMask() const;
		[[nodiscard]] virtual const bgfx::TextureHandle getGPUPick() const;

		virtual void gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback);
		// ------

		static bool supported();
	};
} // namespace rawrbox
