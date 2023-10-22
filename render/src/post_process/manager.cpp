
#include <rawrbox/render/post_process/manager.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/render.hpp>

#include <fmt/format.h>

#include <stdexcept>

namespace rawrbox {
	PostProcessManager::~PostProcessManager() {
		/*for (auto sample : this->_samples)
			RAWRBOX_DESTROY(sample);

		this->_samples.clear();*/
		this->_postProcesses.clear();
	}

	Diligent::RefCntAutoPtr<Diligent::IFramebuffer> PostProcessManager::createFramebuffer(Diligent::ITextureView* renderTarget) {
		const auto& RPDesc = this->_renderPass->GetDesc();
		const auto& SCDesc = rawrbox::render::RENDERER->swapChain()->GetDesc();

#if PLATFORM_MACOS || PLATFORM_IOS || PLATFORM_TVOS
		// In Metal and Vulkan on top of Metal, there are no native subpasses, and
		// attachments can't be preserved between subpasses without saving them to global memory.
		// Thus they can't be memoryless in this usage scenario.
		constexpr auto MemorylessTexBindFlags = Diligent::BIND_NONE;
#else
		const auto MemorylessTexBindFlags = rawrbox::render::RENDERER->device()->GetAdapterInfo().Memory.MemorylessTextureBindFlags;
#endif

		// Create window-size offscreen render target
		Diligent::TextureDesc TexDesc;
		TexDesc.Name = "Color G-buffer";
		TexDesc.Type = Diligent::RESOURCE_DIM_TEX_2D;
		TexDesc.BindFlags = Diligent::BIND_RENDER_TARGET | Diligent::BIND_INPUT_ATTACHMENT;
		TexDesc.Format = RPDesc.pAttachments[0].Format;
		TexDesc.Width = SCDesc.Width;
		TexDesc.Height = SCDesc.Height;
		TexDesc.MipLevels = 1;
		TexDesc.MiscFlags =
		    ((MemorylessTexBindFlags & TexDesc.BindFlags) == TexDesc.BindFlags) ? Diligent::MISC_TEXTURE_FLAG_MEMORYLESS : Diligent::MISC_TEXTURE_FLAG_NONE;

		// Define optimal clear value
		TexDesc.ClearValue.Format = TexDesc.Format;
		TexDesc.ClearValue.Color[0] = 0.F;
		TexDesc.ClearValue.Color[1] = 0.F;
		TexDesc.ClearValue.Color[2] = 0.F;
		TexDesc.ClearValue.Color[3] = 1.F;

		if (pColorBuffer == nullptr)
			rawrbox::render::RENDERER->device()->CreateTexture(TexDesc, nullptr, &pColorBuffer);

		// OpenGL does not allow combining swap chain render target with any
		// other render target, so we have to create an auxiliary texture.
		if (renderTarget == nullptr) {
			TexDesc.Name = "OpenGL Offscreen Render Target";
			TexDesc.Format = SCDesc.ColorBufferFormat;
			TexDesc.MiscFlags = Diligent::MISC_TEXTURE_FLAG_NONE;
			rawrbox::render::RENDERER->device()->CreateTexture(TexDesc, nullptr, &pOpenGLOffsreenColorBuffer);
			renderTarget = pOpenGLOffsreenColorBuffer->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET);
		}

		Diligent::ITextureView* pAttachments[] = //
		    {
			pColorBuffer->GetDefaultView(Diligent::TEXTURE_VIEW_RENDER_TARGET),
			renderTarget //
		    };

		Diligent::FramebufferDesc FBDesc;
		FBDesc.Name = "G-buffer framebuffer";
		FBDesc.pRenderPass = this->_renderPass;
		FBDesc.AttachmentCount = _countof(pAttachments);
		FBDesc.ppAttachments = pAttachments;

		Diligent::RefCntAutoPtr<Diligent::IFramebuffer> pFramebuffer;
		rawrbox::render::RENDERER->device()->CreateFramebuffer(FBDesc, &pFramebuffer);
		VERIFY_EXPR(pFramebuffer != nullptr);

		for (auto& post : this->_postProcesses)
			post->setRTTexture(pColorBuffer->GetDefaultView(Diligent::TEXTURE_VIEW_SHADER_RESOURCE));

		return pFramebuffer;
		/*
				TexDesc.Name = "Depth Z G-buffer";
				TexDesc.Format = RPDesc.pAttachments[1].Format;
				TexDesc.MiscFlags =
				    ((MemorylessTexBindFlags & TexDesc.BindFlags) == TexDesc.BindFlags) ? MISC_TEXTURE_FLAG_MEMORYLESS : MISC_TEXTURE_FLAG_NONE;

				TexDesc.ClearValue.Format = TexDesc.Format;
				TexDesc.ClearValue.Color[0] = 1.f;
				TexDesc.ClearValue.Color[1] = 1.f;
				TexDesc.ClearValue.Color[2] = 1.f;
				TexDesc.ClearValue.Color[3] = 1.f;

				if (!m_GBuffer.pDepthZBuffer)
					rawrbox::render::RENDERER->device()->CreateTexture(TexDesc, nullptr, &m_GBuffer.pDepthZBuffer);

				TexDesc.Name = "Depth buffer";
				TexDesc.Format = RPDesc.pAttachments[2].Format;
				TexDesc.BindFlags = BIND_DEPTH_STENCIL;
				TexDesc.MiscFlags =
				    ((MemorylessTexBindFlags & TexDesc.BindFlags) == TexDesc.BindFlags) ? MISC_TEXTURE_FLAG_MEMORYLESS : MISC_TEXTURE_FLAG_NONE;

				TexDesc.ClearValue.Format = TexDesc.Format;
				TexDesc.ClearValue.DepthStencil.Depth = 1.f;
				TexDesc.ClearValue.DepthStencil.Stencil = 0;

				if (!m_GBuffer.pDepthBuffer)
					rawrbox::render::RENDERER->device()->CreateTexture(TexDesc, nullptr, &m_GBuffer.pDepthBuffer);

				ITextureView* pAttachments[] = //
				    {
					m_GBuffer.pColorBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
					m_GBuffer.pDepthZBuffer->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET),
					m_GBuffer.pDepthBuffer->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL),
					pDstRenderTarget //
				    };

				FramebufferDesc FBDesc;
				FBDesc.Name = "G-buffer framebuffer";
				FBDesc.pRenderPass = m_pRenderPass;
				FBDesc.AttachmentCount = _countof(pAttachments);
				FBDesc.ppAttachments = pAttachments;

				RefCntAutoPtr<IFramebuffer> pFramebuffer;
				m_pDevice->CreateFramebuffer(FBDesc, &pFramebuffer);
				VERIFY_EXPR(pFramebuffer != nullptr);

				// Create SRBs that reference the framebuffer textures

				if (!m_pLightVolumeSRB) {
					m_pLightVolumePSO->CreateShaderResourceBinding(&m_pLightVolumeSRB, true);
					if (auto* pInputColor = m_pLightVolumeSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputColor"))
						pInputColor->Set(m_GBuffer.pColorBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
					if (auto* pInputDepthZ = m_pLightVolumeSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputDepthZ"))
						pInputDepthZ->Set(m_GBuffer.pDepthZBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
				}

				if (!m_pAmbientLightSRB) {
					m_pAmbientLightPSO->CreateShaderResourceBinding(&m_pAmbientLightSRB, true);
					if (auto* pInputColor = m_pAmbientLightSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputColor"))
						pInputColor->Set(m_GBuffer.pColorBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
					if (auto* pInputDepthZ = m_pAmbientLightSRB->GetVariableByName(SHADER_TYPE_PIXEL, "g_SubpassInputDepthZ"))
						pInputDepthZ->Set(m_GBuffer.pDepthZBuffer->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
				}

				return pFramebuffer;*/
	}

	void PostProcessManager::buildRenderPass() {
		const auto& desc = rawrbox::render::RENDERER->swapChain()->GetDesc();

		std::array<Diligent::RenderPassAttachmentDesc, 2> attachments;
		attachments[0].Format = desc.ColorBufferFormat;
		attachments[0].InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
		attachments[0].FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;
		attachments[0].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
		attachments[0].StoreOp = Diligent::ATTACHMENT_STORE_OP_DISCARD;

		attachments[1].Format = desc.ColorBufferFormat;
		attachments[1].InitialState = Diligent::RESOURCE_STATE_RENDER_TARGET;
		attachments[1].FinalState = Diligent::RESOURCE_STATE_RENDER_TARGET;
		attachments[1].LoadOp = Diligent::ATTACHMENT_LOAD_OP_CLEAR;
		attachments[1].StoreOp = Diligent::ATTACHMENT_STORE_OP_STORE;
		/*
				// Subpass 1 - Render G-buffer
				// Subpass 2 - Lighting
				constexpr uint32_t NumSubpasses = 2;

				// Prepar subpass descriptions
				Diligent::SubpassDesc Subpasses[NumSubpasses];

				// Subpass 0 attachments - 2 render targets and depth buffer
				Diligent::AttachmentReference RTAttachmentRefs0[] =
				    {
					{0, Diligent::RESOURCE_STATE_RENDER_TARGET}};

				// Subpass 1 attachments - 1 render target, depth buffer, 2 input attachments
				Diligent::AttachmentReference RTAttachmentRefs1[] =
				    {
					{1, Diligent::RESOURCE_STATE_RENDER_TARGET}};

				Diligent::AttachmentReference InputAttachmentRefs1[] =
				    {
					{0, Diligent::RESOURCE_STATE_INPUT_ATTACHMENT}};

				Subpasses[0].RenderTargetAttachmentCount = _countof(RTAttachmentRefs0);
				Subpasses[0].pRenderTargetAttachments = RTAttachmentRefs0;

				Subpasses[1].RenderTargetAttachmentCount = _countof(RTAttachmentRefs1);
				Subpasses[1].pRenderTargetAttachments = RTAttachmentRefs1;
				Subpasses[1].InputAttachmentCount = _countof(InputAttachmentRefs1);
				Subpasses[1].pInputAttachments = InputAttachmentRefs1;
		*/
		std::vector<Diligent::SubpassDesc> subpasses = {};
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			Diligent::SubpassDesc desc;

			if (i == 0) {
				Diligent::AttachmentReference RTAttachmentRefs0[] =
				    {
					{0, Diligent::RESOURCE_STATE_RENDER_TARGET}};

				desc.RenderTargetAttachmentCount = _countof(RTAttachmentRefs0);
				desc.pRenderTargetAttachments = RTAttachmentRefs0;
			} else {
				Diligent::AttachmentReference InputAttachmentRefs1[] =
				    {
					{0, Diligent::RESOURCE_STATE_INPUT_ATTACHMENT}};

				Diligent::AttachmentReference RTAttachmentRefs0[] =
				    {
					{1, Diligent::RESOURCE_STATE_RENDER_TARGET}};

				desc.RenderTargetAttachmentCount = _countof(RTAttachmentRefs0);
				desc.pRenderTargetAttachments = RTAttachmentRefs0;
				desc.InputAttachmentCount = _countof(InputAttachmentRefs1);
				desc.pInputAttachments = InputAttachmentRefs1;
			}

			subpasses.push_back(desc);
		}

		std::vector<Diligent::SubpassDependencyDesc> deps = {};
		for (size_t i = 0; i < this->_postProcesses.size() - 1; i++) {
			Diligent::SubpassDependencyDesc dep;
			dep.SrcSubpass = i;
			dep.DstSubpass = i + 1;
			dep.SrcStageMask = Diligent::PIPELINE_STAGE_FLAG_RENDER_TARGET;
			dep.DstStageMask = Diligent::PIPELINE_STAGE_FLAG_PIXEL_SHADER;
			dep.SrcAccessMask = Diligent::ACCESS_FLAG_RENDER_TARGET_WRITE;
			dep.DstAccessMask = Diligent::ACCESS_FLAG_SHADER_READ;
			// dep.DstAccessMask = i == 0 ? Diligent::ACCESS_FLAG_SHADER_READ : Diligent::ACCESS_FLAG_INPUT_ATTACHMENT_READ;

			deps.push_back(dep);
		}

		/*Diligent::SubpassDependencyDesc dep;
		dep.SrcSubpass = 0;
		dep.DstSubpass = this->_postProcesses.size();
		dep.SrcStageMask = Diligent::PIPELINE_STAGE_FLAG_RENDER_TARGET;
		dep.DstStageMask = Diligent::PIPELINE_STAGE_FLAG_PIXEL_SHADER;
		dep.SrcAccessMask = Diligent::ACCESS_FLAG_RENDER_TARGET_WRITE;
		dep.DstAccessMask = Diligent::ACCESS_FLAG_SHADER_READ;

		deps.push_back(dep);*/

		Diligent::RenderPassDesc RPDesc;
		RPDesc.Name = "RawrBox::PostProcess::Pass";
		RPDesc.AttachmentCount = static_cast<uint32_t>(attachments.size());
		RPDesc.pAttachments = attachments.data();
		RPDesc.SubpassCount = static_cast<uint32_t>(subpasses.size());
		RPDesc.pSubpasses = subpasses.data();
		RPDesc.DependencyCount = static_cast<uint32_t>(deps.size());
		RPDesc.pDependencies = deps.data();

		if (this->_renderPass != nullptr) this->_renderPass.Release();
		rawrbox::render::RENDERER->device()->CreateRenderPass(RPDesc, &this->_renderPass);
	}

	// Post utils ----
	void PostProcessManager::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to remove {}!", indx));
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase& PostProcessManager::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) throw std::runtime_error(fmt::format("[RawrBox-PostProcess] Failed to get {}!", indx));
		return *this->_postProcesses[indx];
	}

	size_t PostProcessManager::count() {
		return this->_postProcesses.size();
	}
	// ----
	void PostProcessManager::upload() {
		// if (this->_frameBuffer != nullptr) throw std::runtime_error("[RawrBox-PostProcess] Already uploaded!");

		this->buildRenderPass();
		for (size_t indx = 0; indx < this->_postProcesses.size(); indx++) {
			this->_postProcesses[indx]->upload(this->_renderPass, static_cast<uint32_t>(indx));
		}
		/*
				// Create frame buffer ----
				Diligent::ITextureView* pAttachments[] = //
				    {
					rawrbox::RENDERER->getColor(true), _pDstRenderTarget};
				Diligent::FramebufferDesc FBDesc;
				FBDesc.Name = "RawrBox::PostProcess::Frame";
				FBDesc.pRenderPass = this->_renderPass;
				FBDesc.AttachmentCount = 2;
				FBDesc.ppAttachments = pAttachments;

				rawrbox::render::RENDERER->device()->CreateFramebuffer(FBDesc, &this->_frameBuffer);
				// ----

				for (size_t indx = 0; indx < this->_postProcesses.size(); indx++) {
					this->_postProcesses[indx]->upload(this->_renderPass, static_cast<uint32_t>(indx));
				}*/
	}

	Diligent::IFramebuffer* PostProcessManager::getCurrentFramebuffer() {
		auto* pCurrentBackBufferRTV = rawrbox::render::RENDERER->device()->GetDeviceInfo().IsGLDevice() ? nullptr : rawrbox::render::RENDERER->swapChain()->GetCurrentBackBufferRTV();

		auto fb_it = m_FramebufferCache.find(pCurrentBackBufferRTV);
		if (fb_it != m_FramebufferCache.end()) {
			return fb_it->second;
		} else {
			auto it = m_FramebufferCache.emplace(pCurrentBackBufferRTV, this->createFramebuffer(pCurrentBackBufferRTV));
			VERIFY_EXPR(it.second);
			return it.first->second;
		}
	}

	void PostProcessManager::render(Diligent::ITextureView* renderTexture) {
		if (this->_postProcesses.empty()) return;
		if (this->_renderPass == nullptr) return;

		auto context = rawrbox::render::RENDERER->context();
		auto swap = rawrbox::render::RENDERER->swapChain();
		// const auto& frame = this->createFramebuffer(swap->GetCurrentBackBufferRTV());

		Diligent::BeginRenderPassAttribs RPBeginInfo;
		RPBeginInfo.pRenderPass = this->_renderPass;
		RPBeginInfo.pFramebuffer = this->getCurrentFramebuffer();

		std::array<Diligent::OptimizedClearValue, 2> ClearValues;
		// Color
		ClearValues[0].Color[0] = 0.F;
		ClearValues[0].Color[1] = 0.F;
		ClearValues[0].Color[2] = 0.F;
		ClearValues[0].Color[3] = 0.F;

		ClearValues[1].Color[0] = 0.F;
		ClearValues[1].Color[1] = 0.F;
		ClearValues[1].Color[2] = 0.F;
		ClearValues[1].Color[3] = 1.F;

		RPBeginInfo.pClearValues = ClearValues.data();
		RPBeginInfo.ClearValueCount = 2;
		RPBeginInfo.StateTransitionMode = Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

		// Transition all resources to required states as no transitions are allowed within the render pass.
		Diligent::StateTransitionDesc Barriers[] = //
		    {
			{renderTexture->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE} //
		    };

		context->TransitionResourceStates(_countof(Barriers), Barriers);

		context->BeginRenderPass(RPBeginInfo);
		rawrbox::RenderUtils::renderQUAD(renderTexture, false);
		for (size_t i = 0; i < this->_postProcesses.size(); i++) {
			if (i != this->_postProcesses.size() - 1) context->NextSubpass();
			this->_postProcesses[i]->applyEffect();
		}

		context->EndRenderPass();

		/*if (rawrbox::render::RENDERER->device()->GetDeviceInfo().IsGLDevice()) {
			// In OpenGL we now have to copy our off-screen buffer to the default framebuffer
			auto* pOffscreenRenderTarget = frame->GetDesc().ppAttachments[1]->GetTexture();
			auto* pBackBuffer = rawrbox::render::RENDERER->swapChain()->GetCurrentBackBufferRTV()->GetTexture();

			Diligent::CopyTextureAttribs CopyAttribs{pOffscreenRenderTarget, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
			    pBackBuffer, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
			rawrbox::render::RENDERER->context()->CopyTexture(CopyAttribs);
		}*/

		/*if (m_pDevice->GetDeviceInfo().IsGLDevice())
    {
	// In OpenGL we now have to copy our off-screen buffer to the default framebuffer
	auto* pOffscreenRenderTarget = pFramebuffer->GetDesc().ppAttachments[3]->GetTexture();
	auto* pBackBuffer            = m_pSwapChain->GetCurrentBackBufferRTV()->GetTexture();

	CopyTextureAttribs CopyAttribs{pOffscreenRenderTarget, RESOURCE_STATE_TRANSITION_MODE_TRANSITION,
				       pBackBuffer, RESOURCE_STATE_TRANSITION_MODE_TRANSITION};
	m_pImmediateContext->CopyTexture(CopyAttribs);
    }*/

		/*if (this->_samples.empty()) return;

		bgfx::ViewId prevID = rawrbox::CURRENT_VIEW_ID;
		for (size_t pass = 0; pass < this->_postProcesses.size(); pass++) {
			rawrbox::CURRENT_VIEW_ID = rawrbox::POST_PROCESSING_ID + static_cast<bgfx::ViewId>(pass);
			rawrbox::RenderUtils::drawQUAD(pass == 0 ? renderTexture : bgfx::getTexture(this->_samples[pass - 1]), this->_windowSize, false);
			this->_postProcesses[pass]->applyEffect();
		}

		// Draw final texture
		rawrbox::CURRENT_VIEW_ID = prevID;
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		rawrbox::RenderUtils::drawQUAD(bgfx::getTexture(this->_samples.back()), this->_windowSize);*/
	}

} // namespace rawrbox
