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

	std::unique_ptr<rawrbox::StreamingBuffer> IMGUIManager::_pVB;
	std::unique_ptr<rawrbox::StreamingBuffer> IMGUIManager::_pIB;
	// -----------

	// TEXTURE ----
	std::unique_ptr<rawrbox::TextureImage> IMGUIManager::_imguiFontTexture = nullptr;
	// -------
	// ---------

	void IMGUIManager::renderDrawData(ImDrawData* data) {
		if (rawrbox::RENDERER == nullptr) return;
		if (data == nullptr || data->DisplaySize.x <= 0.0F || data->DisplaySize.y <= 0.0F || data->CmdListsCount == 0)
			return;

		auto* context = rawrbox::RENDERER->context();

		// Setup streaming buffer ---
		auto VBOffset = static_cast<uint64_t>(_pVB->allocate(data->TotalVtxCount * sizeof(ImDrawVert), 0));
		auto IBOffset = static_cast<uint64_t>(_pIB->allocate(data->TotalIdxCount * sizeof(ImDrawIdx), 0));

		auto* VertexData = std::bit_cast<ImDrawVert*>(std::bit_cast<uint8_t*>(_pVB->getCPUAddress(0)) + VBOffset);
		auto* IndexData = std::bit_cast<ImDrawIdx*>(std::bit_cast<uint8_t*>(_pIB->getCPUAddress(0)) + IBOffset);

		for (auto* cmdList : data->CmdLists) {
			for (auto& pCmd : cmdList->CmdBuffer) {
				auto* texture = std::bit_cast<rawrbox::TextureBase*>(pCmd.TextureId);
				if (texture == nullptr) texture = rawrbox::MISSING_TEXTURE.get();

				if (pCmd.ElemCount == 0) continue;

				for (uint32_t i = pCmd.IdxOffset; i < pCmd.IdxOffset + pCmd.ElemCount; i++) {
					auto verticeIndex = cmdList->IdxBuffer[i];

					cmdList->VtxBuffer[verticeIndex].textureID = texture->getTextureID();
					cmdList->VtxBuffer[verticeIndex].__padding__.x = static_cast<float>(texture->getSlice());
				}
			}

			std::memcpy(VertexData, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size * sizeof(ImDrawVert));
			std::memcpy(IndexData, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

			VertexData += cmdList->VtxBuffer.Size;
			IndexData += cmdList->IdxBuffer.Size;
		}

		_pVB->release(0);
		_pIB->release(0);
		//  -------------------

		auto setup = [&]() {
			auto* buffer = _pVB->buffer();

			context->SetVertexBuffers(0, 1, &buffer, &VBOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION, Diligent::SET_VERTEX_BUFFERS_FLAG_RESET);
			context->SetIndexBuffer(_pIB->buffer(), IBOffset, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->SetPipelineState(_2dPipeline);
		};

		setup();

		// Render command lists
		uint32_t GlobalIdxOffset = 0;
		uint32_t GlobalVtxOffset = 0;

		for (const auto& pCmdList : data->CmdLists) {
			for (const auto& pCmd : pCmdList->CmdBuffer) {
				if (pCmd.UserCallback != nullptr) {
					// NOLINTBEGIN(*)
					if (pCmd.UserCallback == ImDrawCallback_ResetRenderState) {
						// NOLINTEND(*)
						setup();
						continue;
					}

					pCmd.UserCallback(pCmdList, &pCmd);
					continue;
				}

				if (pCmd.ElemCount == 0) continue;

				// Apply scissor/clipping rectangle
				Diligent::Rect Scissor(
				    static_cast<uint32_t>((pCmd.ClipRect.x - data->DisplayPos.x) * data->FramebufferScale.x),
				    static_cast<uint32_t>((pCmd.ClipRect.y - data->DisplayPos.y) * data->FramebufferScale.y),
				    static_cast<uint32_t>((pCmd.ClipRect.z - data->DisplayPos.x) * data->FramebufferScale.x),
				    static_cast<uint32_t>((pCmd.ClipRect.w - data->DisplayPos.y) * data->FramebufferScale.y));

				if (!Scissor.IsValid()) continue;
				rawrbox::RENDERER->context()->SetScissorRects(1, &Scissor, 0, 0);

				Diligent::DrawIndexedAttribs DrawAttrs = {pCmd.ElemCount, Diligent::VT_UINT32, Diligent::DRAW_FLAG_VERIFY_STATES};
				DrawAttrs.FirstIndexLocation = GlobalIdxOffset + pCmd.IdxOffset;
				DrawAttrs.BaseVertex = GlobalVtxOffset + pCmd.VtxOffset;

				rawrbox::RENDERER->context()->DrawIndexed(DrawAttrs);
			}

			GlobalIdxOffset += pCmdList->IdxBuffer.Size;
			GlobalVtxOffset += pCmdList->VtxBuffer.Size;
		}

		_pVB->flush(0);
		_pIB->flush(0);
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
		_pVB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::VertexBuffer", Diligent::BIND_VERTEX_BUFFER, MaxVertsInStreamingBuffer * static_cast<uint32_t>(sizeof(ImDrawVert)), 1);
		_pVB->setPersistent(true);

		_pIB = std::make_unique<rawrbox::StreamingBuffer>("RawrBox::Stencil::IndexBuffer", Diligent::BIND_INDEX_BUFFER, MaxVertsInStreamingBuffer * 3 * static_cast<uint32_t>(sizeof(ImDrawIdx)), 1);
		_pIB->setPersistent(true);
		// --------------------------

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
