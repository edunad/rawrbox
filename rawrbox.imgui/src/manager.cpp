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
		for (const auto& cmd : data->CmdLists) {
			for (const auto& buffer : cmd->CmdBuffer) {
				if (buffer.UserCallback != nullptr) {
					buffer.UserCallback(cmd, &buffer);
					continue;
				}

				if (buffer.ElemCount == 0) continue;

				auto* texture = std::bit_cast<rawrbox::TextureBase*>(buffer.TextureId);
				if (texture == nullptr) texture = _imguiFontTexture.get();

				std::vector<rawrbox::PosUVColorVertexData> verts(cmd->VtxBuffer.size() - buffer.VtxOffset);
				std::memcpy(verts.data(), cmd->VtxBuffer.Data + buffer.VtxOffset, verts.size() * sizeof(rawrbox::PosUVColorVertexData));

				std::vector<uint32_t> indc(cmd->IdxBuffer.size() - buffer.IdxOffset);
				std::memcpy(indc.data(), cmd->IdxBuffer.Data + buffer.IdxOffset, indc.size() * sizeof(uint32_t));

				for (auto& vert : verts) {
					vert.textureID = texture->getTextureID();
					vert.uv.z = static_cast<float>(texture->getSlice());
				}

				rawrbox::AABBu clip =
				    {
					static_cast<uint32_t>((buffer.ClipRect.x - data->DisplayPos.x) * data->FramebufferScale.x),
					static_cast<uint32_t>((buffer.ClipRect.y - data->DisplayPos.y) * data->FramebufferScale.y),
					static_cast<uint32_t>((buffer.ClipRect.z - data->DisplayPos.x) * data->FramebufferScale.x),
					static_cast<uint32_t>((buffer.ClipRect.w - data->DisplayPos.y) * data->FramebufferScale.y)};

				stencil->pushClipping({clip, true});
				stencil->drawVertices(verts, indc);
				stencil->popClipping();
			}
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
		IO.BackendRendererName = "RawrBox-IMGUI";
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		IO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

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
		_imguiFontTexture.reset();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext();
	}
	//------
} // namespace rawrbox
