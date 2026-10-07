#if defined(_WIN32) && defined(RAWRBOX_SUPPORT_VULKAN)
	#include <rawrbox/render/utils/swapchain.hpp>
	#include <rawrbox/utils/logger.hpp>

	#include <CommandQueueVk.h>
	#include <vulkan/vulkan_win32.h>

	#include <algorithm>
	#include <limits>

namespace rawrbox {
	// FUNCTIONS ---
	bool SwapChainVkFunctions::load(VkInstance instance, VkDevice device) {
		HMODULE loader = GetModuleHandleA("vulkan-1.dll"); // Diligent already loaded it (i hope)
		if (loader == nullptr) {
			loader = LoadLibraryA("vulkan-1.dll");
			this->library = loader;
		}

		if (loader == nullptr) return false;

		auto getInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(loader, "vkGetInstanceProcAddr"));
		if (getInstanceProcAddr == nullptr) return false;

		auto getDeviceProcAddr = reinterpret_cast<PFN_vkGetDeviceProcAddr>(getInstanceProcAddr(instance, "vkGetDeviceProcAddr"));
		if (getDeviceProcAddr == nullptr) return false;

		// Instance ---
		this->getSurfaceCapabilities = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		this->getSurfaceFormats = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		this->getPresentModes = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
		this->getSurfaceSupport = reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(getInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
		this->createWin32Surface = getInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR");
		this->destroySurface = reinterpret_cast<PFN_vkDestroySurfaceKHR>(getInstanceProcAddr(instance, "vkDestroySurfaceKHR"));
		// ----

		// Device ---
		this->createSwapchain = reinterpret_cast<PFN_vkCreateSwapchainKHR>(getDeviceProcAddr(device, "vkCreateSwapchainKHR"));
		this->destroySwapchain = reinterpret_cast<PFN_vkDestroySwapchainKHR>(getDeviceProcAddr(device, "vkDestroySwapchainKHR"));
		this->getSwapchainImages = reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(getDeviceProcAddr(device, "vkGetSwapchainImagesKHR"));
		this->acquireNextImage = reinterpret_cast<PFN_vkAcquireNextImageKHR>(getDeviceProcAddr(device, "vkAcquireNextImageKHR"));

		this->createSemaphore = reinterpret_cast<PFN_vkCreateSemaphore>(getDeviceProcAddr(device, "vkCreateSemaphore"));
		this->destroySemaphore = reinterpret_cast<PFN_vkDestroySemaphore>(getDeviceProcAddr(device, "vkDestroySemaphore"));
		this->createFence = reinterpret_cast<PFN_vkCreateFence>(getDeviceProcAddr(device, "vkCreateFence"));
		this->destroyFence = reinterpret_cast<PFN_vkDestroyFence>(getDeviceProcAddr(device, "vkDestroyFence"));
		this->waitForFences = reinterpret_cast<PFN_vkWaitForFences>(getDeviceProcAddr(device, "vkWaitForFences"));
		this->resetFences = reinterpret_cast<PFN_vkResetFences>(getDeviceProcAddr(device, "vkResetFences"));
		// ----

		return this->getSurfaceCapabilities != nullptr && this->getSurfaceFormats != nullptr && this->getPresentModes != nullptr && this->getSurfaceSupport != nullptr &&
		       this->createWin32Surface != nullptr && this->destroySurface != nullptr &&
		       this->createSwapchain != nullptr && this->destroySwapchain != nullptr && this->getSwapchainImages != nullptr && this->acquireNextImage != nullptr &&
		       this->createSemaphore != nullptr && this->destroySemaphore != nullptr && this->createFence != nullptr && this->destroyFence != nullptr &&
		       this->waitForFences != nullptr && this->resetFences != nullptr;
	}

	void SwapChainVkFunctions::unload() {
		if (this->library == nullptr) return;

		FreeLibrary(this->library);
		this->library = nullptr;
	}
	// ---------

	// PRIVATE ---
	VkFormat SwapChainVk::getBufferFormat(Diligent::TEXTURE_FORMAT format) {
		switch (format) {
			case Diligent::TEX_FORMAT_RGBA8_UNORM:
				return VK_FORMAT_R8G8B8A8_UNORM;

			case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB:
				return VK_FORMAT_R8G8B8A8_SRGB;

			case Diligent::TEX_FORMAT_BGRA8_UNORM:
				return VK_FORMAT_B8G8R8A8_UNORM;

			case Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB:
				return VK_FORMAT_B8G8R8A8_SRGB;

			case Diligent::TEX_FORMAT_RGBA16_FLOAT:
				return VK_FORMAT_R16G16B16A16_SFLOAT;

			default:
				RAWRBOX_CRITICAL("Unsupported transparent swap chain format '{}'", static_cast<int>(format));
		}
	}

	Diligent::TEXTURE_FORMAT SwapChainVk::getSwappedFormat(Diligent::TEXTURE_FORMAT format) {
		switch (format) {
			case Diligent::TEX_FORMAT_RGBA8_UNORM:
				return Diligent::TEX_FORMAT_BGRA8_UNORM;

			case Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB:
				return Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB;

			case Diligent::TEX_FORMAT_BGRA8_UNORM:
				return Diligent::TEX_FORMAT_RGBA8_UNORM;

			case Diligent::TEX_FORMAT_BGRA8_UNORM_SRGB:
				return Diligent::TEX_FORMAT_RGBA8_UNORM_SRGB;

			default:
				return Diligent::TEX_FORMAT_UNKNOWN;
		}
	}

	void SwapChainVk::createSurface() {
		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = GetModuleHandleA(nullptr);
		surfaceInfo.hwnd = this->_hwnd;

		auto createWin32Surface = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(this->_vk.createWin32Surface);
		if (createWin32Surface(this->_device->GetVkInstance(), &surfaceInfo, nullptr, &this->_surface) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to create Vulkan window surface");
	}

	void SwapChainVk::recreateSurface() {
		this->destroySwapChain();

		this->_vk.destroySurface(this->_device->GetVkInstance(), this->_surface, nullptr);
		this->_surface = VK_NULL_HANDLE;

		this->createSurface();
	}

	void SwapChainVk::queryCapabilities(VkSurfaceCapabilitiesKHR& caps) {
		auto* physicalDevice = this->_device->GetVkPhysicalDevice();

		VkResult result = this->_vk.getSurfaceCapabilities(physicalDevice, this->_surface, &caps);
		if (result == VK_ERROR_SURFACE_LOST_KHR) {
			this->recreateSurface();
			result = this->_vk.getSurfaceCapabilities(physicalDevice, this->_surface, &caps);
		}

		if (result != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to query the Vulkan surface capabilities ({})", static_cast<int>(result));
	}

	VkSurfaceFormatKHR SwapChainVk::selectFormat() {
		auto* physicalDevice = this->_device->GetVkPhysicalDevice();

		uint32_t formatCount = 0;
		if (this->_vk.getSurfaceFormats(physicalDevice, this->_surface, &formatCount, nullptr) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to query the Vulkan surface formats");

		std::vector<VkSurfaceFormatKHR> formats(formatCount);
		if (this->_vk.getSurfaceFormats(physicalDevice, this->_surface, &formatCount, formats.data()) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to query the Vulkan surface formats");

		if (formats.size() == 1 && formats.front().format == VK_FORMAT_UNDEFINED) {
			return {getBufferFormat(this->_desc.ColorBufferFormat), formats.front().colorSpace};
		}

		auto find = [&formats](VkFormat format) -> const VkSurfaceFormatKHR* {
			auto fnd = std::ranges::find_if(formats, [format](const VkSurfaceFormatKHR& f) { return f.format == format; });
			return fnd == formats.end() ? nullptr : &(*fnd);
		};

		if (const auto* fnd = find(getBufferFormat(this->_desc.ColorBufferFormat)); fnd != nullptr) return *fnd;

		auto swapped = getSwappedFormat(this->_desc.ColorBufferFormat);
		if (swapped != Diligent::TEX_FORMAT_UNKNOWN) {
			if (const auto* fnd = find(getBufferFormat(swapped)); fnd != nullptr) {
				this->_desc.ColorBufferFormat = swapped;
				return *fnd;
			}
		}

		RAWRBOX_CRITICAL("Transparent swap chain format '{}' not supported by the surface", static_cast<int>(this->_desc.ColorBufferFormat));
	}

	void SwapChainVk::createSwapChain(const VkSurfaceCapabilitiesKHR& caps) {
		auto* physicalDevice = this->_device->GetVkPhysicalDevice();

		// Size ---
		if (caps.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			this->_desc.Width = caps.currentExtent.width;
			this->_desc.Height = caps.currentExtent.height;
		} else {
			this->_desc.Width = std::clamp(this->_desc.Width, caps.minImageExtent.width, caps.maxImageExtent.width);
			this->_desc.Height = std::clamp(this->_desc.Height, caps.minImageExtent.height, caps.maxImageExtent.height);
		}
		// ----

		const VkSurfaceFormatKHR surfaceFormat = this->selectFormat();

		// Present mode ---
		uint32_t modeCount = 0;
		if (this->_vk.getPresentModes(physicalDevice, this->_surface, &modeCount, nullptr) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to query the Vulkan present modes");

		std::vector<VkPresentModeKHR> modes(modeCount);
		if (this->_vk.getPresentModes(physicalDevice, this->_surface, &modeCount, modes.data()) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to query the Vulkan present modes");

		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; // Always supported (vsync)
		if (!this->_vsync) {
			if (std::ranges::find(modes, VK_PRESENT_MODE_MAILBOX_KHR) != modes.end()) {
				presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
			} else if (std::ranges::find(modes, VK_PRESENT_MODE_IMMEDIATE_KHR) != modes.end()) {
				presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			}
		}
		// ----

		if ((caps.supportedUsageFlags & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) == 0) RAWRBOX_CRITICAL("Vulkan surface does not support rendering into the swap chain");
		const VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | (caps.supportedUsageFlags & (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT));

		uint32_t imageCount = std::max(this->_desc.BufferCount, caps.minImageCount);
		if (caps.maxImageCount > 0) imageCount = std::min(imageCount, caps.maxImageCount);

		VkSwapchainKHR oldSwapChain = this->_swapChain;

		VkSwapchainCreateInfoKHR info = {};
		info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		info.surface = this->_surface;
		info.minImageCount = imageCount;
		info.imageFormat = surfaceFormat.format;
		info.imageColorSpace = surfaceFormat.colorSpace;
		info.imageExtent = {this->_desc.Width, this->_desc.Height};
		info.imageArrayLayers = 1;
		info.imageUsage = usage;
		info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		info.compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
		info.presentMode = presentMode;
		info.clipped = VK_TRUE;
		info.oldSwapchain = oldSwapChain;

		auto* device = this->_device->GetVkDevice();
		if (this->_vk.createSwapchain(device, &info, nullptr, &this->_swapChain) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to create transparent Vulkan swap chain");
		if (oldSwapChain != VK_NULL_HANDLE) this->_vk.destroySwapchain(device, oldSwapChain, nullptr);

		this->createBuffers();
	}

	void SwapChainVk::createBuffers() {
		auto* device = this->_device->GetVkDevice();

		uint32_t imageCount = 0;
		this->_vk.getSwapchainImages(device, this->_swapChain, &imageCount, nullptr);
		std::vector<VkImage> images(imageCount);
		this->_vk.getSwapchainImages(device, this->_swapChain, &imageCount, images.data());

		this->_desc.BufferCount = imageCount;

		// Back buffers ---
		for (uint32_t i = 0; i < imageCount; i++) {
			Diligent::TextureDesc texDesc;
			texDesc.Name = "RawrBox::Transparent::BackBuffer";
			texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
			texDesc.Width = this->_desc.Width;
			texDesc.Height = this->_desc.Height;
			texDesc.Format = this->_desc.ColorBufferFormat;
			texDesc.BindFlags = Diligent::BIND_RENDER_TARGET;
			texDesc.Usage = Diligent::USAGE_DEFAULT;
			texDesc.MipLevels = 1;

			Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
			this->_device->CreateTextureFromVulkanImage(images[i], texDesc, Diligent::RESOURCE_STATE_UNDEFINED, &texture);
			if (texture == nullptr) RAWRBOX_CRITICAL("Failed to wrap swap chain image {}", i);

			this->_backBufferRTV.emplace_back(texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET));

			VkSemaphoreCreateInfo semInfo = {};
			semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

			VkSemaphore semaphore = VK_NULL_HANDLE;
			if (this->_vk.createSemaphore(device, &semInfo, nullptr, &semaphore) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to create swap chain semaphore");
			this->_renderFinished.push_back(semaphore);
		}
		// ----

		this->createDepthBuffer(this->_desc.Width, this->_desc.Height);
	}

	void SwapChainVk::createFallbackBuffers() {
		Diligent::TextureDesc texDesc;
		texDesc.Name = "RawrBox::Transparent::FallbackBuffer";
		texDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		texDesc.Width = 1;
		texDesc.Height = 1;
		texDesc.Format = this->_desc.ColorBufferFormat;
		texDesc.BindFlags = Diligent::BIND_RENDER_TARGET;
		texDesc.Usage = Diligent::USAGE_DEFAULT;
		texDesc.MipLevels = 1;

		Diligent::RefCntAutoPtr<Diligent::ITexture> texture;
		this->_device->CreateTexture(texDesc, nullptr, &texture);
		if (texture == nullptr) RAWRBOX_CRITICAL("Failed to create swap chain fallback buffer");

		this->_fallbackRTV = texture->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
		this->createDepthBuffer(1, 1);
	}

	void SwapChainVk::createDepthBuffer(Diligent::Uint32 width, Diligent::Uint32 height) {
		if (this->_desc.DepthBufferFormat == Diligent::TEX_FORMAT_UNKNOWN) return;

		Diligent::TextureDesc depthDesc;

		depthDesc.Name = "RawrBox::Transparent::DepthBuffer";
		depthDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;

		depthDesc.Width = width;
		depthDesc.Height = height;

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

	void SwapChainVk::release() {
		this->_context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		this->_context->Flush();

		this->_backBufferRTV.clear();
		this->_fallbackRTV.Release();
		this->_depthBufferDSV.Release();

		this->_imageIndex = 0;
		this->_imageAcquired = false;

		this->_device->IdleGPU();
		this->_device->ReleaseStaleResources(true);

		auto* device = this->_device->GetVkDevice();
		for (auto* semaphore : this->_renderFinished) {
			this->_vk.destroySemaphore(device, semaphore, nullptr);
		}

		this->_renderFinished.clear();
	}

	void SwapChainVk::destroySwapChain() {
		this->release();

		if (this->_swapChain != VK_NULL_HANDLE) {
			this->_vk.destroySwapchain(this->_device->GetVkDevice(), this->_swapChain, nullptr);
			this->_swapChain = VK_NULL_HANDLE;
		}
	}

	bool SwapChainVk::acquire() {
		this->_imageAcquired = false;
		if (this->_swapChain == VK_NULL_HANDLE || this->_minimized) return false;

		auto* device = this->_device->GetVkDevice();

		VkResult result = this->_vk.acquireNextImage(device, this->_swapChain, std::numeric_limits<uint64_t>::max(), VK_NULL_HANDLE, this->_acquireFence, &this->_imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_ERROR_SURFACE_LOST_KHR) return false;
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) RAWRBOX_CRITICAL("Failed to acquire swap chain image ({})", static_cast<int>(result));

		this->_vk.waitForFences(device, 1, &this->_acquireFence, VK_TRUE, std::numeric_limits<uint64_t>::max());
		this->_vk.resetFences(device, 1, &this->_acquireFence);

		this->_imageAcquired = true;
		return true;
	}

	void SwapChainVk::recreate() {
		VkSurfaceCapabilitiesKHR caps = {};
		this->queryCapabilities(caps);

		this->_minimized = caps.currentExtent.width == 0 || caps.currentExtent.height == 0;
		if (this->_minimized) {
			if (this->_backBufferRTV.empty() && this->_fallbackRTV == nullptr) this->createFallbackBuffers();
			return;
		}

		this->release();
		this->createSwapChain(caps);
		if (this->acquire()) return;

		this->queryCapabilities(caps);
		if (caps.currentExtent.width == 0 || caps.currentExtent.height == 0) return;

		this->release();
		this->createSwapChain(caps);
		this->acquire();
	}
	// -----------

	SwapChainVk::SwapChainVk(Diligent::IReferenceCounters* refCounters, Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, HWND hwnd, bool vsync) : TBase(refCounters), _device(device, Diligent::IID_RenderDeviceVk), _context(context), _desc(desc), _hwnd(hwnd), _vsync(vsync) {
		if (this->_device == nullptr) RAWRBOX_CRITICAL("Transparent Vulkan swap chain requires a Vulkan device");
		if (hwnd == nullptr) RAWRBOX_CRITICAL("Invalid window handle");
		if (!this->_vk.load(this->_device->GetVkInstance(), this->_device->GetVkDevice())) RAWRBOX_CRITICAL("Failed to load the Vulkan swap chain functions");

		this->_desc.PreTransform = Diligent::SURFACE_TRANSFORM_IDENTITY;
		this->createSurface();

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		if (this->_vk.createFence(this->_device->GetVkDevice(), &fenceInfo, nullptr, &this->_acquireFence) != VK_SUCCESS) RAWRBOX_CRITICAL("Failed to create swap chain fence");

		this->recreate();
	}

	SwapChainVk::~SwapChainVk() {
		this->destroySwapChain();

		auto* device = this->_device->GetVkDevice();
		if (this->_acquireFence != VK_NULL_HANDLE) this->_vk.destroyFence(device, this->_acquireFence, nullptr);
		if (this->_surface != VK_NULL_HANDLE) this->_vk.destroySurface(this->_device->GetVkInstance(), this->_surface, nullptr);

		this->_vk.unload();
	}

	bool SwapChainVk::isSupported(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, HWND hwnd) {
		Diligent::RefCntAutoPtr<Diligent::IRenderDeviceVk> deviceVk(device, Diligent::IID_RenderDeviceVk);
		if (deviceVk == nullptr || context == nullptr || hwnd == nullptr) return false;

		rawrbox::SwapChainVkFunctions vk = {};
		if (!vk.load(deviceVk->GetVkInstance(), deviceVk->GetVkDevice())) {
			vk.unload();
			return false;
		}

		VkWin32SurfaceCreateInfoKHR surfaceInfo = {};
		surfaceInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceInfo.hinstance = GetModuleHandleA(nullptr);
		surfaceInfo.hwnd = hwnd;

		VkSurfaceKHR surface = VK_NULL_HANDLE;
		auto createWin32Surface = reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(vk.createWin32Surface);
		if (createWin32Surface(deviceVk->GetVkInstance(), &surfaceInfo, nullptr, &surface) != VK_SUCCESS) {
			vk.unload();
			return false;
		}

		auto* queue = static_cast<Diligent::ICommandQueueVk*>(context->LockCommandQueue());
		const uint32_t queueFamily = queue->GetQueueFamilyIndex();
		context->UnlockCommandQueue();

		VkBool32 canPresent = VK_FALSE;
		bool supported = vk.getSurfaceSupport(deviceVk->GetVkPhysicalDevice(), queueFamily, surface, &canPresent) == VK_SUCCESS && canPresent == VK_TRUE;

		// Pre-multiplied alpha ---
		VkSurfaceCapabilitiesKHR caps = {};
		supported = supported && vk.getSurfaceCapabilities(deviceVk->GetVkPhysicalDevice(), surface, &caps) == VK_SUCCESS;
		supported = supported && (caps.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) != 0;
		// ----

		vk.destroySurface(deviceVk->GetVkInstance(), surface, nullptr);
		vk.unload();

		return supported;
	}

	void SwapChainVk::QueryInterface(const Diligent::INTERFACE_ID& IID, Diligent::IObject** ppInterface) {
		if (ppInterface == nullptr) return;

		if (IID == Diligent::IID_SwapChain) {
			*ppInterface = this;
			(*ppInterface)->AddRef();
		} else {
			TBase::QueryInterface(IID, ppInterface);
		}
	}

	void SwapChainVk::Present(Diligent::Uint32 SyncInterval) {
		VkResult result = VK_SUCCESS;

		if (this->_imageAcquired) {
			// Rendering done
			auto* rtv = this->GetCurrentBackBufferRTV();
			this->_context->SetRenderTargets(0, nullptr, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);

			Diligent::StateTransitionDesc barrier{rtv->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_PRESENT, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE};
			this->_context->TransitionResourceStates(1, &barrier);
			this->_context->Flush();
			// ----

			auto* queue = static_cast<Diligent::ICommandQueueVk*>(this->_context->LockCommandQueue());

			VkSemaphore renderFinished = this->_renderFinished[this->_imageIndex];

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &renderFinished;
			queue->Submit(submitInfo);

			VkPresentInfoKHR presentInfo = {};
			presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &renderFinished;
			presentInfo.swapchainCount = 1;
			presentInfo.pSwapchains = &this->_swapChain;
			presentInfo.pImageIndices = &this->_imageIndex;

			result = queue->Present(presentInfo);
			this->_context->UnlockCommandQueue();
			// ----

			this->_imageAcquired = false;
			if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR && result != VK_ERROR_OUT_OF_DATE_KHR && result != VK_ERROR_SURFACE_LOST_KHR) RAWRBOX_CRITICAL("Failed to present transparent swap chain ({})", static_cast<int>(result));
		} else {
			this->_context->Flush(); // Minimized
		}

		this->_context->FinishFrame();
		this->_device->ReleaseStaleResources();

		// Next image ---
		const bool vsync = SyncInterval > 0;
		const bool vsyncChanged = vsync != this->_vsync;
		this->_vsync = vsync;

		if (result != VK_SUCCESS || vsyncChanged || this->_minimized || this->_swapChain == VK_NULL_HANDLE) {
			this->recreate();
			return;
		}

		if (!this->acquire()) this->recreate();
		// ----
	}

	const Diligent::SwapChainDesc& SwapChainVk::GetDesc() const {
		return this->_desc;
	}

	void SwapChainVk::Resize(Diligent::Uint32 width, Diligent::Uint32 height, Diligent::SURFACE_TRANSFORM /*transform*/) {
		if (width == 0 || height == 0) return; // Minimized, handled on present
		if (width == this->_desc.Width && height == this->_desc.Height && this->_swapChain != VK_NULL_HANDLE && !this->_minimized) return;

		this->_desc.Width = width;
		this->_desc.Height = height;

		this->recreate();
	}

	void SwapChainVk::SetFullscreenMode(const Diligent::DisplayModeAttribs& /*DisplayMode*/) {}
	void SwapChainVk::SetWindowedMode() {}
	void SwapChainVk::SetMaximumFrameLatency(Diligent::Uint32 /*MaxLatency*/) {}

	Diligent::ITextureView* SwapChainVk::GetCurrentBackBufferRTV() {
		if (this->_imageIndex < this->_backBufferRTV.size()) return this->_backBufferRTV[this->_imageIndex];
		return this->_fallbackRTV;
	}

	Diligent::ITextureView* SwapChainVk::GetDepthBufferDSV() { return this->_depthBufferDSV; }

	// UTILS ---
	bool SwapChainUtils::createVk(Diligent::IRenderDevice* device, Diligent::IDeviceContext* context, const Diligent::SwapChainDesc& desc, const Diligent::NativeWindow& window, bool vsync, Diligent::ISwapChain** swapChain) {
		auto* hwnd = static_cast<HWND>(window.hWnd);
		if (!SwapChainVk::isSupported(device, context, hwnd)) return false;

		auto* chain = Diligent::MakeNewRCObj<SwapChainVk>()(device, context, desc, hwnd, vsync);
		chain->QueryInterface(Diligent::IID_SwapChain, reinterpret_cast<Diligent::IObject**>(swapChain));
		return true;
	}
	// ---------
} // namespace rawrbox
#endif
