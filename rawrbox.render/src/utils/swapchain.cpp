#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_DX12)
	#include <rawrbox/render/utils/swapchain.hpp>
	#include <rawrbox/utils/logger.hpp>

	#include <CommandQueueD3D12.h>

	#include <algorithm>

	#pragma comment(lib, "dcomp.lib")
	#pragma comment(lib, "dxgi.lib")

namespace rawrbox {
	// PRIVATE ---
	DXGI_FORMAT SwapChainDCompD3D12::getBufferFormat(Diligent::TEXTURE_FORMAT format) {
		switch (format) {
			case Diligent::TEX_FORMAT_RGBA8_UNORM:
			case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB:
				return DXGI_FORMAT_R8G8B8A8_UNORM;

			case Diligent::TEX_FORMAT_BGRA8_UNORM:
			case Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB:
				return DXGI_FORMAT_B8G8R8A8_UNORM;

			case Diligent::TEX_FORMAT_RGBA16_FLOAT:
				return DXGI_FORMAT_R16G16B16A16_FLOAT;

			default:
				RAWRBOX_CRITICAL("Unsupported composition swap chain format '{}'", static_cast<int>(format));
		}
	}

	void SwapChainDCompD3D12::createBuffers() {
		for (Diligent::Uint32 i = 0; i < this->_desc.BufferCount; i++) {
			Microsoft::WRL::ComPtr<ID3D12Resource> buffer;

			if (FAILED(this->_swapChain->GetBuffer(i, IID_PPV_ARGS(&buffer)))) RAWRBOX_CRITICAL("Failed to get swap chain buffer {}", i);
			buffer->SetName(L"RawrBox Back Buffer");

			Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
			this->_device->CreateTextureFromD3DResource(buffer.Get(), Diligent::RESOURCE_STATE_UNDEFINED, &texture);
			if (texture == nullptr) RAWRBOX_CRITICAL("Failed to wrap swap chain buffer {}", i);

			Diligent::TextureViewDesc rtvDesc;
			rtvDesc.ViewType = Diligent::TEXTURE_VIEW_RENDER_TARGET;
			rtvDesc.Format = this->_desc.ColorBufferFormat;

			Diligent::RefCntAutoPtr<Diligent::ITextureView> rtv;
			texture->CreateView(rtvDesc, &rtv);
			this->_backBufferRTV.emplace_back(rtv);
		}

		if (this->_desc.DepthBufferFormat != Diligent::TEX_FORMAT_UNKNOWN) {
			Diligent::TextureDesc depthDesc;

			depthDesc.Name = "RawrBox DComp depth buffer";
			depthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;

			depthDesc.Width = this->_desc.Width;
			depthDesc.Height = this->_desc.Height;

			depthDesc.Format = this->_desc.DepthBufferFormat;

			depthDesc.Usage = Diligent::USAGE_DEFAULT;
			depthDesc.BindFlags = Diligent::BIND_DEPTH_STENCIL;

			depthDesc.ClearValue.Format = this->_desc.DepthBufferFormat;
			depthDesc.ClearValue.DepthStencil.Depth = this->_desc.DefaultDepthValue;
			depthDesc.ClearValue.DepthStencil.Stencil = this->_desc.DefaultStencilValue;

			Diligent::RefCntAutoPtr<Diligent::ITexture> depth;
			this->_device->CreateTexture(depthDesc, nullptr, &depth);
			if (depth == nullptr) RAWRBOX_CRITICAL("Failed to create swap chain depth buffer");

			this->_depthBufferDSV = depth->GetDefaultView(Diligent::TEXTURE_VIEW_DEPTH_STENCIL);
		}
	}

	void SwapChainDCompD3D12::release() {
		this->_context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		this->_context->Flush();

		this->_backBufferRTV.clear();
		this->_depthBufferDSV.Release();

		this->_device->IdleGPU();
	}
	// -----------

	SwapChainDCompD3D12::SwapChainDCompD3D12(Diligent::IReferenceCounters* refCounters, Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, HWND hwnd) : TBase(refCounters), _device(device, Diligent::IID_RenderDeviceD3D12), _context(context), _desc(desc) {
		if (this->_device == nullptr) RAWRBOX_CRITICAL("Composition swap chain requires a D3D12 device");
		if (hwnd == nullptr) RAWRBOX_CRITICAL("Invalid window handle");

		// Size ---
		if (this->_desc.Width == 0 || this->_desc.Height == 0) {
			RECT rc = {};
			GetClientRect(hwnd, &rc);

			this->_desc.Width = static_cast<Diligent::Uint32>(std::max(rc.right - rc.left, 1L));
			this->_desc.Height = static_cast<Diligent::Uint32>(std::max(rc.bottom - rc.top, 1L));
		}

		this->_desc.PreTransform = Diligent::SURFACE_TRANSFORM_IDENTITY;
		// ----

		// Swap chain ---
		Microsoft::WRL::ComPtr<IDXGIFactory2> factory;
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)))) RAWRBOX_CRITICAL("Failed to create DXGI factory");

		DXGI_SWAP_CHAIN_DESC1 scDesc = {};
		scDesc.Width = this->_desc.Width;
		scDesc.Height = this->_desc.Height;
		scDesc.Format = getBufferFormat(this->_desc.ColorBufferFormat);
		scDesc.SampleDesc.Count = 1;
		scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		scDesc.BufferCount = this->_desc.BufferCount;

		scDesc.Scaling = DXGI_SCALING_STRETCH;
		scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
		scDesc.AlphaMode = DXGI_ALPHA_MODE_PREMULTIPLIED;

		Microsoft::WRL::ComPtr<IDXGISwapChain1> swapChain1;

		auto* queue = static_cast<Diligent::ICommandQueueD3D12*>(this->_context->LockCommandQueue());
		HRESULT hr = factory->CreateSwapChainForComposition(queue->GetD3D12CommandQueue(), &scDesc, nullptr, &swapChain1);
		this->_context->UnlockCommandQueue();

		if (FAILED(hr)) RAWRBOX_CRITICAL("Failed to create composition swap chain (0x{:08X})", static_cast<uint32_t>(hr));
		if (FAILED(swapChain1.As(&this->_swapChain))) RAWRBOX_CRITICAL("IDXGISwapChain3 not supported");
		// ----

		// Bind ---
		if (FAILED(DCompositionCreateDevice(nullptr, IID_PPV_ARGS(&this->_dcompDevice)))) RAWRBOX_CRITICAL("Failed to create DirectComposition device");
		if (FAILED(this->_dcompDevice->CreateTargetForHwnd(hwnd, TRUE, &this->_dcompTarget))) RAWRBOX_CRITICAL("Failed to create DirectComposition target");
		if (FAILED(this->_dcompDevice->CreateVisual(&this->_dcompVisual))) RAWRBOX_CRITICAL("Failed to create DirectComposition visual");

		this->_dcompVisual->SetContent(this->_swapChain.Get());
		this->_dcompTarget->SetRoot(this->_dcompVisual.Get());

		if (FAILED(this->_dcompDevice->Commit())) RAWRBOX_CRITICAL("Failed to commit DirectComposition tree");
		// ----

		this->createBuffers();
	}

	SwapChainDCompD3D12::~SwapChainDCompD3D12() {
		this->release();
	}

	void SwapChainDCompD3D12::QueryInterface(const Diligent::INTERFACE_ID& IID, Diligent::IObject** ppInterface) {
		if (ppInterface == nullptr) return;

		if (IID == Diligent::IID_SwapChain) {
			*ppInterface = this;
			(*ppInterface)->AddRef();
		} else {
			TBase::QueryInterface(IID, ppInterface);
		}
	}

	void SwapChainDCompD3D12::Present(Diligent::Uint32 SyncInterval) {
		auto* rtv = this->GetCurrentBackBufferRTV();
		this->_context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

		Diligent::StateTransitionDesc barrier{rtv->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_PRESENT, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
		this->_context->TransitionResourceStates(1, &barrier);
		this->_context->Flush();

		HRESULT hr = this->_swapChain->Present(SyncInterval, 0);
		if (FAILED(hr)) RAWRBOX_CRITICAL("Composition swap chain present failed (0x{:08X})", static_cast<uint32_t>(hr));

		this->_context->FinishFrame();
		this->_device->ReleaseStaleResources();
	}

	const Diligent::SwapChainDesc& SwapChainDCompD3D12::GetDesc() const {
		return this->_desc;
	}

	void SwapChainDCompD3D12::Resize(Diligent::Uint32 width, Diligent::Uint32 height, Diligent::SURFACE_TRANSFORM /*transform*/) {
		if (width == 0 || height == 0) return; // Minimized
		if (width == this->_desc.Width && height == this->_desc.Height) return;

		this->_desc.Width = width;
		this->_desc.Height = height;

		this->release();
		if (FAILED(this->_swapChain->ResizeBuffers(this->_desc.BufferCount, width, height, DXGI_FORMAT_UNKNOWN, 0))) RAWRBOX_CRITICAL("Failed to resize composition swap chain");
		this->createBuffers();
	}

	void SwapChainDCompD3D12::SetFullscreenMode(const Diligent::DisplayModeAttribs& /*DisplayMode*/) {}
	void SwapChainDCompD3D12::SetWindowedMode() {}
	void SwapChainDCompD3D12::SetMaximumFrameLatency(Diligent::Uint32 /*MaxLatency*/) {}

	Diligent::ITextureView* SwapChainDCompD3D12::GetCurrentBackBufferRTV() { return this->_backBufferRTV[this->_swapChain->GetCurrentBackBufferIndex()]; }
	Diligent::ITextureView* SwapChainDCompD3D12::GetDepthBufferDSV() { return this->_depthBufferDSV; }

	// UTILS ---
	void SwapChainUtils::create(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, const Diligent::NativeWindow& window, Diligent::ISwapChain** swapChain) {
		auto* chain = Diligent::MakeNewRCObj<SwapChainDCompD3D12>()(device, context, desc, static_cast<HWND>(window.hWnd));
		chain->QueryInterface(Diligent::IID_SwapChain, reinterpret_cast<Diligent::IObject**>(swapChain));
	}
	// ---------
} // namespace rawrbox
#endif
