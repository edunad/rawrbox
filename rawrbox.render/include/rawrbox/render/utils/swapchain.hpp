#pragma once

#include <NativeWindow.h>
#include <RenderDevice.h>
#include <SwapChain.h>

#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_DX12)
	// Must be included before the Diligent D3D12 interfaces
	// clang-format off
	#include <d3d12.h>
	#include <dcomp.h>
	#include <dxgi1_4.h>
	#include <wrl/client.h>
// clang-format on

	#include <ObjectBase.hpp>
	#include <RefCntAutoPtr.hpp>

	#include <RenderDeviceD3D12.h>

	#include <vector>
#endif

namespace rawrbox {
#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_DX12)
	class SwapChainDCompD3D12 : public Diligent::ObjectBase<Diligent::ISwapChain> {
	protected:
		using TBase = Diligent::ObjectBase<Diligent::ISwapChain>;

		Diligent::RefCntAutoPtr<Diligent::IRenderDeviceD3D12> _device;
		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> _context;
		Diligent::SwapChainDesc _desc = {};

		Microsoft::WRL::ComPtr<IDXGISwapChain3> _swapChain;
		Microsoft::WRL::ComPtr<IDCompositionDevice> _dcompDevice;
		Microsoft::WRL::ComPtr<IDCompositionTarget> _dcompTarget;
		Microsoft::WRL::ComPtr<IDCompositionVisual> _dcompVisual;

		std::vector<Diligent::RefCntAutoPtr<Diligent::ITextureView>> _backBufferRTV = {};
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _depthBufferDSV;

		static DXGI_FORMAT getBufferFormat(Diligent::TEXTURE_FORMAT format);

		void createBuffers();
		void release();

	public:
		SwapChainDCompD3D12(Diligent::IReferenceCounters* refCounters, Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, HWND hwnd);
		SwapChainDCompD3D12(const SwapChainDCompD3D12&) = delete;
		SwapChainDCompD3D12(SwapChainDCompD3D12&&) = delete;
		SwapChainDCompD3D12& operator=(const SwapChainDCompD3D12&) = delete;
		SwapChainDCompD3D12& operator=(SwapChainDCompD3D12&&) = delete;
		~SwapChainDCompD3D12() override;

		void DILIGENT_CALL_TYPE QueryInterface(const Diligent::INTERFACE_ID& IID, Diligent::IObject** ppInterface) override;

		void DILIGENT_CALL_TYPE Present(Diligent::Uint32 SyncInterval) override;
		[[nodiscard]] const Diligent::SwapChainDesc& DILIGENT_CALL_TYPE GetDesc() const override;
		void DILIGENT_CALL_TYPE Resize(Diligent::Uint32 width, Diligent::Uint32 height, Diligent::SURFACE_TRANSFORM transform) override;

		void DILIGENT_CALL_TYPE SetFullscreenMode(const Diligent::DisplayModeAttribs& DisplayMode) override;
		void DILIGENT_CALL_TYPE SetWindowedMode() override;

		void DILIGENT_CALL_TYPE SetMaximumFrameLatency(Diligent::Uint32 MaxLatency) override;

		[[nodiscard]] Diligent::ITextureView* DILIGENT_CALL_TYPE GetCurrentBackBufferRTV() override;
		[[nodiscard]] Diligent::ITextureView* DILIGENT_CALL_TYPE GetDepthBufferDSV() override;
	};
#endif

	class SwapChainUtils {
	public:
#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_DX12)
		static void create(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, const Diligent::NativeWindow& window, Diligent::ISwapChain** swapChain);
#endif
	};
} // namespace rawrbox
