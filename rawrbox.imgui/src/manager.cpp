#include <rawrbox/engine/engine.hpp>
#include <rawrbox/imgui/manager.hpp>
#include <rawrbox/math/utils/math.hpp>

// ---- IMGUI ----
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
// ---------------

namespace rawrbox {
	// PROTECTED ---
	// TEXTURE ----
	std::unique_ptr<rawrbox::TextureImage> IMGUIManager::_imguiFontTexture = nullptr;
	// -------
	// ---------

	void IMGUIManager::renderDrawData(ImDrawData* data) {
		if (rawrbox::RENDERER == nullptr) return;
		if (data == nullptr || data->DisplaySize.x <= 0.0F || data->DisplaySize.y <= 0.0F || data->CmdListsCount == 0)
			return;

		auto* stencil = rawrbox::RENDERER->stencil();

		for (const auto& pCmdList : data->CmdLists) {
			std::vector<uint32_t> indices(pCmdList->IdxBuffer.Size);
			std::memcpy(indices.data(), pCmdList->IdxBuffer.Data, pCmdList->IdxBuffer.Size * sizeof(ImDrawIdx));

			std::vector<rawrbox::PosUVColorVertexData> vertices(pCmdList->VtxBuffer.Size);
			std::memcpy(vertices.data(), pCmdList->VtxBuffer.Data, pCmdList->VtxBuffer.Size * sizeof(ImDrawVert));

			for (const auto& pCmd : pCmdList->CmdBuffer) {
				if (pCmd.UserCallback != nullptr) {
					// NOLINTBEGIN(*)
					if (pCmd.UserCallback == ImDrawCallback_ResetRenderState) {
						// NOLINTEND(*)
						continue; // Not supported
					}

					pCmd.UserCallback(pCmdList, &pCmd);
					continue;
				}

				if (pCmd.ElemCount == 0) continue;

				// Texturing ----
				auto* texture = pCmd.TextureId;
				if (texture == nullptr) texture = rawrbox::MISSING_TEXTURE.get();

				for (uint32_t i = pCmd.IdxOffset; i < pCmd.IdxOffset + pCmd.ElemCount; i++) {
					auto verticeIndex = indices[i];

					vertices[verticeIndex].textureID = texture->getTextureID();
					vertices[verticeIndex].uv.z = static_cast<float>(texture->getSlice());
				}
				// ------------

				// Apply scissor/clipping rectangle
				rawrbox::AABBu Scissor(
				    static_cast<uint32_t>((pCmd.ClipRect.x - data->DisplayPos.x) * data->FramebufferScale.x),
				    static_cast<uint32_t>((pCmd.ClipRect.y - data->DisplayPos.y) * data->FramebufferScale.y),
				    static_cast<uint32_t>((pCmd.ClipRect.z - data->DisplayPos.x) * data->FramebufferScale.x),
				    static_cast<uint32_t>((pCmd.ClipRect.w - data->DisplayPos.y) * data->FramebufferScale.y));

				if (!Scissor.valid()) continue;

				stencil->pushOptimize(false);
				stencil->pushClipping({Scissor, true});

				stencil->drawVertices(vertices, indices);

				stencil->popClipping();
				stencil->popOptimize();
			}
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

		IO.Fonts->TexID = _imguiFontTexture.get();
	}

	void IMGUIManager::clear() {
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void IMGUIManager::render() {
		ImGui::Render();
		renderDrawData(ImGui::GetDrawData());
	}

	void IMGUIManager::shutdown() {
		_imguiFontTexture.reset();

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	//------
} // namespace rawrbox
