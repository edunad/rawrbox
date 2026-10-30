#include <rawrbox/engine/engine.hpp>
#include <rawrbox/imgui/manager.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/utils/barrier.hpp>

// ---- IMGUI ----
#include <tracy/Tracy.hpp>

#include <backends/imgui_impl_glfw.h>
#include <imgui.h>
// ---------------

#ifdef RAWRBOX_IMGUI_IMPLOT
	#include <implot.h>
#endif

#ifdef RAWRBOX_IMGUI_RICHMD
	#include <rawrbox/render/resources/texture.hpp>
	#include <rawrbox/resources/manager.hpp>
	#include <rawrbox/utils/vfs.hpp>

	#include <imgui_rich_md/rich_md.h>
	#include <imgui_rich_md/rich_md_host.h>
#endif

namespace rawrbox {
	// PROTECTED ---
	// TEXTURE ----
	std::unordered_map<int, std::unique_ptr<rawrbox::TextureImage>> IMGUIManager::_textures = {};
	// -------
	// ---------

	void IMGUIManager::renderDrawData(ImDrawData* data) {
		if (rawrbox::RENDERER == nullptr) return;
		if (data == nullptr || data->DisplaySize.x <= 0.0F || data->DisplaySize.y <= 0.0F || data->CmdLists.empty())
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
				const auto* texture = pCmd.GetTexID();
				if (texture == nullptr) texture = rawrbox::MISSING_TEXTURE.get();

				for (uint32_t i = pCmd.IdxOffset; i < pCmd.IdxOffset + pCmd.ElemCount; i++) {
					const auto verticeIndex = indices[i];

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
		const auto* window = rawrbox::Window::getWindow();
		const auto& renderer = window->getRenderer();

		// SETUP IMGUI
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		darkTheme ? ImGui::StyleColorsDark() : ImGui::StyleColorsLight();

#ifdef RAWRBOX_IMGUI_IMPLOT
		ImPlot::CreateContext();
		darkTheme ? ImPlot::StyleColorsDark() : ImPlot::StyleColorsLight();
#endif

		ImGuiIO& IO = ImGui::GetIO();
		IO.BackendRendererName = "RawrBox-IMGUI";
		IO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
		IO.BackendFlags |= ImGuiBackendFlags_RendererHasTextures;

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

#ifdef RAWRBOX_IMGUI_RICHMD
		RichMd::HostServices services = {};
		services.ReadAsset = [](const std::string& path) -> RichMd::AssetBytes {
			auto data = rawrbox::VFS::read(path);
			if (data.empty()) return RichMd::ReadAssetDefault(path);

			return data;
		};

		RichMd::MarkdownOptions options = {};
		options.callbacks.OnImage = [](const std::string& path) -> std::optional<RichMd::MarkdownImage> {
			if (!rawrbox::RESOURCES::isLoaded(path)) {
				const auto ext = std::filesystem::path(path).extension().generic_string();
				const bool canLoad = std::ranges::any_of(rawrbox::RESOURCES::getLoaders(), [&ext](const auto& loader) {
					return dynamic_cast<rawrbox::TextureLoader*>(loader.get()) != nullptr && loader->canLoad(ext);
				});

				if (!canLoad || !rawrbox::VFS::contains(path)) return RichMd::OnImage_Default(path);
			}

			auto* texture = rawrbox::RESOURCES::loadFile<rawrbox::ResourceTexture>(path)->get();
			const auto size = texture->getSize().cast<float>();

			return RichMd::MarkdownImage{
			    .texture_id = texture,
			    .size = {size.x, size.y},

			    .uv0 = {0.F, 0.F},
			    .uv1 = {1.F, 1.F},

			    .col_tint = {1.F, 1.F, 1.F, 1.F},
			    .col_border = {0.F, 0.F, 0.F, 0.F}};
		};

		RichMd::SetHostServices(services);
		RichMd::CreateContext(options);
#endif
	}

	void IMGUIManager::clear() {
		ZoneScopedN("IMGUIManager::clear");
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void IMGUIManager::render() {
		ZoneScopedN("IMGUIManager::render");
		ImGui::Render();

		auto* data = ImGui::GetDrawData();

		// Textures ----
		for (ImTextureData* tex : *data->Textures) {
			switch (tex->Status) {
				case ImTextureStatus_WantCreate:
					{
						if (tex->Format != ImTextureFormat_RGBA32) RAWRBOX_CRITICAL("Unsupported IMGUI texture format, only RGBA32 is supported");

						auto texture = std::make_unique<rawrbox::TextureImage>(rawrbox::Vector2u(tex->Width, tex->Height), static_cast<const uint8_t*>(tex->GetPixels()), uint8_t(4));
						texture->setName("IMGUI");
						texture->upload(Diligent::TEX_FORMAT_UNKNOWN, rawrbox::TextureFlags::DYNAMIC);

						tex->SetTexID(texture.get());
						tex->SetStatus(ImTextureStatus_OK);

						_textures[tex->UniqueID] = std::move(texture);
						break;
					}
				case ImTextureStatus_WantUpdates:
					{
						auto* handle = _textures.at(tex->UniqueID)->getTexture();
						auto* context = rawrbox::RENDERER->context();

						rawrbox::BarrierUtils::barrier({{handle, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
						for (const auto& rect : tex->Updates) {
							Diligent::Box UpdateBox;
							UpdateBox.MinX = rect.x;
							UpdateBox.MinY = rect.y;
							UpdateBox.MaxX = rect.x + rect.w;
							UpdateBox.MaxY = rect.y + rect.h;

							Diligent::TextureSubResData SubresData;
							SubresData.Stride = static_cast<uint64_t>(tex->GetPitch());
							SubresData.pData = tex->GetPixelsAt(rect.x, rect.y);

							context->UpdateTexture(handle, 0, 0, UpdateBox, SubresData, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
						}
						rawrbox::BarrierUtils::barrier({{handle, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

						tex->SetStatus(ImTextureStatus_OK);
						break;
					}
				case ImTextureStatus_WantDestroy:
					{
						if (tex->UnusedFrames == 0) break;

						_textures.erase(tex->UniqueID);
						tex->SetTexID(ImTextureID_Invalid);
						tex->SetStatus(ImTextureStatus_Destroyed);
						break;
					}
				default:
					break;
			}
		}
		// ------------

		renderDrawData(data);
	}

	void IMGUIManager::shutdown(rawrbox::ENGINE_THREADS thread) {
		if (thread == rawrbox::ENGINE_THREADS::THREAD_RENDER) {
#ifdef RAWRBOX_IMGUI_RICHMD
			RichMd::DestroyContext();
#endif

			for (ImTextureData* tex : ImGui::GetPlatformIO().Textures) {
				tex->SetTexID(ImTextureID_Invalid);
				tex->SetStatus(ImTextureStatus_Destroyed);
			}

			_textures.clear();
		} else {
			ImGui_ImplGlfw_Shutdown();
#ifdef RAWRBOX_IMGUI_IMPLOT
			ImPlot::DestroyContext();
#endif
			ImGui::DestroyContext();
		}
	}
	//------
} // namespace rawrbox