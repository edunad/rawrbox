#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/render/texture/render.hpp>

#include <Common/interface/RefCntAutoPtr.hpp>

#include <Graphics/GraphicsEngine/interface/DeviceContext.h>
#include <Graphics/GraphicsEngine/interface/RenderDevice.h>
#include <Graphics/GraphicsEngine/interface/SwapChain.h>

#include <functional>

namespace rawrbox {

	class RendererBase {
	protected:
		std::unique_ptr<rawrbox::TextureRender> _render = nullptr;
		// std::unique_ptr<rawrbox::TextureRender> _decals = nullptr;
		// bgfx::TextureHandle _GPUBlitTex = BGFX_INVALID_HANDLE;

		rawrbox::Colorf _clearColor = rawrbox::Colors::Black();
		rawrbox::Vector2i _size = {};

		bool _vsync = false;

		// GPU PICKING ----
		// uint32_t _gpuReadFrame = 0;
		// std::array<uint8_t, rawrbox::GPU_PICK_SAMPLE_SIZE> _gpuPixelData = {};
		// std::vector<std::function<void(uint32_t)>> _gpuPickCallbacks = {};
		// ----------------

		virtual void clear();
		virtual void frame();
		virtual void gpuCheck();

	public:
		std::function<void()> worldRender = nullptr;
		std::function<void()> overlayRender = nullptr;
		std::function<void()> postRender = nullptr;

		// HANDLES ---
		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> context;
		Diligent::RefCntAutoPtr<Diligent::ISwapChain> swapChain;
		Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device;
		// ----

		RendererBase(const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
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
		//[[nodiscard]] virtual const bgfx::TextureHandle getDepth() const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getColor() const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getMask() const;
		//[[nodiscard]] virtual const bgfx::TextureHandle getGPUPick() const;

		[[nodiscard]] virtual const rawrbox::Vector2i getSize() const;
		[[nodiscard]] virtual bool getVSync() const;
		virtual void setVSync(bool vsync);
		virtual void gpuPick(const rawrbox::Vector2i& pos, std::function<void(uint32_t)> callback);
		// ------

		[[nodiscard]] virtual bool supported() const;
	};
} // namespace rawrbox
