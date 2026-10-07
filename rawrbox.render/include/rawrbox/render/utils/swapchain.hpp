#pragma once

#include <NativeWindow.h>
#include <RenderDevice.h>
#include <SwapChain.h>

#if defined(_WIN32) && (defined(RAWRBOX_SUPPORT_DX12) || defined(RAWRBOX_SUPPORT_VULKAN))
	#include <ObjectBase.hpp>
	#include <RefCntAutoPtr.hpp>

	#include <vector>
#endif

#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_DX12)
// clang-format off
	#include <d3d12.h>
	#include <dcomp.h>
	#include <dxgi1_4.h>
	#include <wrl/client.h>
// clang-format on

	#include <RenderDeviceD3D12.h>
#endif

#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_VULKAN)
// clang-format off
	#include <Windows.h>
	#include <vulkan/vulkan.h>
// clang-format on

	#include <RenderDeviceVk.h>
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

#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_VULKAN)
	struct SwapChainVkFunctions {
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR getSurfaceCapabilities = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR getSurfaceFormats = nullptr;
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR getPresentModes = nullptr;
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR getSurfaceSupport = nullptr;
		PFN_vkVoidFunction createWin32Surface = nullptr; // PFN_vkCreateWin32SurfaceKHR
		PFN_vkDestroySurfaceKHR destroySurface = nullptr;

		PFN_vkCreateSwapchainKHR createSwapchain = nullptr;
		PFN_vkDestroySwapchainKHR destroySwapchain = nullptr;
		PFN_vkGetSwapchainImagesKHR getSwapchainImages = nullptr;
		PFN_vkAcquireNextImageKHR acquireNextImage = nullptr;

		PFN_vkCreateSemaphore createSemaphore = nullptr;
		PFN_vkDestroySemaphore destroySemaphore = nullptr;
		PFN_vkCreateFence createFence = nullptr;
		PFN_vkDestroyFence destroyFence = nullptr;
		PFN_vkWaitForFences waitForFences = nullptr;
		PFN_vkResetFences resetFences = nullptr;

		HMODULE library = nullptr;

		bool load(VkInstance instance, VkDevice device);
		void unload();
	};

	class SwapChainVk : public Diligent::ObjectBase<Diligent::ISwapChain> {
	protected:
		using TBase = Diligent::ObjectBase<Diligent::ISwapChain>;

		Diligent::RefCntAutoPtr<Diligent::IRenderDeviceVk> _device;
		Diligent::RefCntAutoPtr<Diligent::IDeviceContext> _context;
		Diligent::SwapChainDesc _desc = {};

		HWND _hwnd = nullptr;
		rawrbox::SwapChainVkFunctions _vk = {};

		VkSurfaceKHR _surface = VK_NULL_HANDLE;
		VkSwapchainKHR _swapChain = VK_NULL_HANDLE;
		VkFence _acquireFence = VK_NULL_HANDLE;

		std::vector<VkSemaphore> _renderFinished = {}; // Per image

		uint32_t _imageIndex = 0;
		bool _imageAcquired = false;
		bool _minimized = false;
		bool _vsync = true;

		std::vector<Diligent::RefCntAutoPtr<Diligent::ITextureView>> _backBufferRTV = {};
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _fallbackRTV;
		Diligent::RefCntAutoPtr<Diligent::ITextureView> _depthBufferDSV;

		static VkFormat getBufferFormat(Diligent::TEXTURE_FORMAT format);
		static Diligent::TEXTURE_FORMAT getSwappedFormat(Diligent::TEXTURE_FORMAT format); // RGBA <-> BGRA

		void createSurface();
		void recreateSurface();
		void queryCapabilities(VkSurfaceCapabilitiesKHR& caps);
		VkSurfaceFormatKHR selectFormat();

		void createSwapChain(const VkSurfaceCapabilitiesKHR& caps);
		void createBuffers();
		void createFallbackBuffers();
		void createDepthBuffer(Diligent::Uint32 width, Diligent::Uint32 height);
		void destroySwapChain();
		void release();

		bool acquire();
		void recreate();

	public:
		SwapChainVk(Diligent::IReferenceCounters* refCounters, Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, HWND hwnd, bool vsync);
		SwapChainVk(const SwapChainVk&) = delete;
		SwapChainVk(SwapChainVk&&) = delete;
		SwapChainVk& operator=(const SwapChainVk&) = delete;
		SwapChainVk& operator=(SwapChainVk&&) = delete;
		~SwapChainVk() override;

		static bool isSupported(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, HWND hwnd);

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

#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_VULKAN)
		static bool createVk(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, const Diligent::NativeWindow& window, bool vsync, Diligent::ISwapChain** swapChain);
#endif
	};
} // namespace rawrbox
