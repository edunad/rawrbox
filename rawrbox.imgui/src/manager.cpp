#include <rawrbox/engine/engine.hpp>
#include <rawrbox/imgui/manager.hpp>
#include <rawrbox/math/utils/math.hpp>

// ---- IMGUI ----
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
// ---------------

namespace rawrbox {
	// PROTECTED ---
	// Buffers ---
	Diligent::IPipelineState* IMGUIManager::_2dPipeline = nullptr;

	std::unique_ptr<Diligent::DynamicBuffer> IMGUIManager::_pVB;
	std::unique_ptr<Diligent::DynamicBuffer> IMGUIManager::_pIB;
	// -----------

	// TEXTURE ----
	std::unique_ptr<rawrbox::TextureImage> IMGUIManager::_imguiFontTexture = nullptr;
	// -------
	// ---------

	void IMGUIManager::renderDrawData(ImDrawData* data) {
		if (rawrbox::RENDERER == nullptr) return;
		if (data == nullptr || data->DisplaySize.x <= 0.0F || data->DisplaySize.y <= 0.0F || data->CmdListsCount == 0)
			return;

		auto* device = rawrbox::RENDERER->device();
		auto* context = rawrbox::RENDERER->context();

		// Check & Resize Buffers ---
		uint64_t vtxSize = sizeof(ImDrawVert) * data->TotalVtxCount;
		if (vtxSize > _pVB->GetDesc().Size) {
			int totalVtx = rawrbox::MathUtils::nextPow2(data->TotalVtxCount);
			_pVB->Resize(device, context, sizeof(ImDrawVert) * static_cast<uint64_t>(totalVtx), true);
		}

		uint64_t idxSize = sizeof(ImDrawIdx) * data->TotalIdxCount;
		if (idxSize > _pIB->GetDesc().Size) {
			int totalIdx = rawrbox::MathUtils::nextPow2(data->TotalIdxCount);
			_pIB->Resize(device, context, sizeof(ImDrawIdx) * static_cast<uint64_t>(totalIdx), true);
		}
		// ----------------

		// Setup data ----
		{
			Diligent::MapHelper<ImDrawVert> Vertex(context, _pVB->GetBuffer(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
			Diligent::MapHelper<ImDrawIdx> Index(context, _pIB->GetBuffer(), Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

			ImDrawVert* pVtxDst = Vertex;
			ImDrawIdx* pIdxDst = Index;

			for (int CmdListID = 0; CmdListID < data->CmdListsCount; CmdListID++) {
				const ImDrawList* pCmdList = data->CmdLists[CmdListID];

				memcpy(pVtxDst, pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(pIdxDst, pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

				pVtxDst += pCmdList->VtxBuffer.Size;
				pIdxDst += pCmdList->IdxBuffer.Size;
			}
		}
		// -------

		auto setup = [&]() //
		{
			// Setup shader and vertex buffers
			std::array<Diligent::IBuffer*, 1> pVBs = {_pVB->GetBuffer()};

			context->SetVertexBuffers(0, 1, pVBs.data(), nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(_pIB->GetBuffer(), 0, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->SetPipelineState(_2dPipeline);
		};

		setup();

		// Render command lists
		// (Because we merged all buffers into a single one, we maintain our own offset into them)
		uint32_t GlobalIdxOffset = 0;
		uint32_t GlobalVtxOffset = 0;

		for (int CmdListID = 0; CmdListID < data->CmdListsCount; CmdListID++) {
			ImDrawList* pCmdList = data->CmdLists[CmdListID];

			for (int CmdID = 0; CmdID < pCmdList->CmdBuffer.Size; CmdID++) {
				ImDrawCmd* pCmd = &pCmdList->CmdBuffer[CmdID];
				if (pCmd->UserCallback != nullptr) {
					// User callback, registered via ImDrawList::AddCallback()
					// (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
					if (pCmd->UserCallback == ImDrawCallback_ResetRenderState)
						setup();
					else
						pCmd->UserCallback(pCmdList, pCmd);
				} else {
					if (pCmd->ElemCount == 0)
						continue;

					auto* texture = std::bit_cast<rawrbox::TextureBase*>(pCmd->TextureId);
					if (texture == nullptr) texture = _imguiFontTexture.get();

					// Apply scissor/clipping rectangle
					Diligent::Rect Scissor(
					    (pCmd->ClipRect.x - data->DisplayPos.x) * data->FramebufferScale.x,
					    (pCmd->ClipRect.y - data->DisplayPos.y) * data->FramebufferScale.y,
					    (pCmd->ClipRect.z - data->DisplayPos.x) * data->FramebufferScale.x,
					    (pCmd->ClipRect.w - data->DisplayPos.y) * data->FramebufferScale.y //
					);

					if (!Scissor.IsValid())
						continue;

					rawrbox::RENDERER->context()->SetScissorRects(1, &Scissor, 0, 0);

					for (int i = pCmd->VtxOffset; i < pCmdList->VtxBuffer.size(); i++) {
						auto& vert = pCmdList->VtxBuffer[i];

						vert.textureID = texture->getTextureID();
						vert.__padding__.x = static_cast<float>(texture->getSlice());
					}

					Diligent::DrawIndexedAttribs DrawAttrs{pCmd->ElemCount, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_STATES};
					DrawAttrs.FirstIndexLocation = pCmd->IdxOffset + GlobalIdxOffset;
					DrawAttrs.BaseVertex = pCmd->VtxOffset + GlobalVtxOffset;

					rawrbox::RENDERER->context()->DrawIndexed(DrawAttrs);
				}
			}

			GlobalIdxOffset += pCmdList->IdxBuffer.Size;
			GlobalVtxOffset += pCmdList->VtxBuffer.Size;
		}
	}
	//------

	// PROTECTED ---
	void IMGUIManager::init(bool darkTheme) {
		auto* window = rawrbox::Window::getWindow();
		auto& renderer = window->getRenderer();

		// SETUP IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		darkTheme ? ImGui::StyleColorsDark() : ImGui::StyleColorsLight();

		ImGuiIO& IO = ImGui::GetIO();
		IO.BackendRendererName = "RawrBox-IMGUI";
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		switch (renderer.getRenderType()) {
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
				ImGui_ImplGlfw_InitForOther(window->getGLFWHandle(), true);
				break;
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				ImGui_ImplGlfw_InitForVulkan(window->getGLFWHandle(), true);
				break;
			default:
				throw std::runtime_error("Not supported");
				break;
		}
		// ----

		// Setup buffers ---
		{
			Diligent::BufferDesc VBDesc;
			VBDesc.Name = "RawrBox::Buffer:IMGUI::VERTEX";
			VBDesc.BindFlags = Diligent::BIND_VERTEX_BUFFER;
			VBDesc.Size = 1024 * sizeof(ImDrawVert);
			VBDesc.Usage = Diligent::USAGE_DYNAMIC;
			VBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = VBDesc;

			_pVB = std::make_unique<Diligent::DynamicBuffer>(renderer.device(), dynamicBuff);
		}

		{
			Diligent::BufferDesc VBDesc;
			VBDesc.Name = "RawrBox::Buffer:IMGUI::INDEX";
			VBDesc.ElementByteStride = sizeof(ImDrawIdx);
			VBDesc.BindFlags = Diligent::BIND_INDEX_BUFFER;
			VBDesc.Size = 2048 * sizeof(ImDrawIdx);
			VBDesc.Usage = Diligent::USAGE_DYNAMIC;
			VBDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;

			Diligent::DynamicBufferCreateInfo dynamicBuff;
			dynamicBuff.Desc = VBDesc;

			_pIB = std::make_unique<Diligent::DynamicBuffer>(renderer.device(), dynamicBuff);
		}
		// -----------------

		_2dPipeline = rawrbox::PipelineUtils::getPipeline("Stencil::2D"); // Re-use stencil pipeline
		if (_2dPipeline == nullptr) throw std::runtime_error("Failed to get stencil pipeline, call imgui init after initializing the renderer");
	}

	void IMGUIManager::load() {
		// Load font --
		ImGuiIO& IO = ImGui::GetIO();

		unsigned char* pData = nullptr;
		rawrbox::Vector2i size = {};
		IO.Fonts->GetTexDataAsRGBA32(&pData, &size.x, &size.y);

		_imguiFontTexture = std::make_unique<rawrbox::TextureImage>(size.cast<uint32_t>(), pData, uint8_t(4));
		_imguiFontTexture->setName("IMGUI");
		_imguiFontTexture->upload();
		// ----------

		// NOLINTBEGIN(*)
		IO.Fonts->TexID = (ImTextureID)_imguiFontTexture.get();
		// NOLINTEND(*)
	}

	void IMGUIManager::clear() {
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		ImGui::DockSpaceOverViewport(ImGui::GetWindowDockID(), ImGui::GetMainViewport());
	}

	void IMGUIManager::render() {
		ImGui::Render();
		renderDrawData(ImGui::GetDrawData());
	}

	void IMGUIManager::shutdown() {
		_pIB.reset();
		_pVB.reset();

		_imguiFontTexture.reset();

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	//------
} // namespace rawrbox
