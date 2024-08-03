#include <rawrbox/engine/engine.hpp>
#include <rawrbox/imgui/manager.hpp>

// ---- IMGUI ----
#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
// ---------------

namespace rawrbox {
	// PROTECTED ---
	// BUFFERS ----
	std::unique_ptr<rawrbox::TextureImage> IMGUIManager::_imguiFontTexture = nullptr;
	// -------

	rawrbox::Window* IMGUIManager::_window = nullptr;
	rawrbox::RendererBase* IMGUIManager::_renderer = nullptr;

	void IMGUIManager::renderDrawData(ImDrawData* data) {
		if (data == nullptr || data->DisplaySize.x <= 0.0F || data->DisplaySize.y <= 0.0F || data->CmdListsCount == 0) {
			return;
		}

		auto* stencil = _renderer->stencil();
		for (auto& cmd : data->CmdLists) {
			std::vector<rawrbox::PosUVColorVertexData> verts(cmd->VtxBuffer.size());
			std::memcpy(verts.data(), cmd->VtxBuffer.Data, cmd->VtxBuffer.size() * sizeof(rawrbox::PosUVColorVertexData));

			std::vector<uint32_t> indc(cmd->IdxBuffer.Data, cmd->IdxBuffer.Data + cmd->IdxBuffer.size());
			for (auto& cmdB : cmd->CmdBuffer) {
				auto* texture = std::bit_cast<rawrbox::TextureBase*>(cmdB.TextureId);
				if (texture == nullptr) continue;

				if (cmdB.UserCallback != nullptr) {
					cmdB.UserCallback(cmd, &cmdB);
					continue;
				}

				for (uint32_t i = 0; i < cmdB.ElemCount; i++) {
					auto idx = indc[cmdB.IdxOffset + i];
					auto& vert = verts[idx];

					vert.textureID = texture->getTextureID();
					vert.uv.z = static_cast<float>(texture->getSlice());
				}
			}

			// TODO: stencil->pushClipping({});
			stencil->drawVertices(verts, indc);
			// TODO: stencil->popClipping();
		}
	}
	//------

	// PROTECTED ---
	void IMGUIManager::init(rawrbox::Window& window, bool darkTheme) {
		_window = &window;
		_renderer = &window.getRenderer();

		// Init ImGui
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		darkTheme ? ImGui::StyleColorsDark() : ImGui::StyleColorsLight();

		ImGuiIO& IO = ImGui::GetIO();
		IO.BackendRendererName = "RawrBox-Renderer";
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

		switch (_renderer->getRenderType()) {
			case Diligent::RENDER_DEVICE_TYPE_D3D12:
				ImGui_ImplGlfw_InitForOther(_window->getGLFWHandle(), true);
				break;
			case Diligent::RENDER_DEVICE_TYPE_VULKAN:
				ImGui_ImplGlfw_InitForVulkan(_window->getGLFWHandle(), true);
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

		_imguiFontTexture = std::make_unique<rawrbox::TextureImage>(size.cast<uint32_t>(), pData, 4U);
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
