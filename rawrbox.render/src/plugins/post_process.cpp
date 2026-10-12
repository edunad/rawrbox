#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/cameras/base.hpp>
#include <rawrbox/render/plugins/post_process.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/utils/barrier.hpp>

#include <algorithm>

namespace rawrbox {
	PostProcessPlugin::~PostProcessPlugin() {
		this->_postProcesses.clear();
		this->_swapTexture.reset();

		RAWRBOX_DESTROY(this->_buffer);
	}

	void PostProcessPlugin::initialize(const rawrbox::Vector2u& /*size*/) {
		if (this->_buffer != nullptr) RAWRBOX_CRITICAL("Plugin already initialized!");

		Diligent::BufferDesc BuffPixelDesc;
		BuffPixelDesc.Name = "rawrbox::PostProcess";
		BuffPixelDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffPixelDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffPixelDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffPixelDesc.Size = sizeof(rawrbox::BindlessPostProcessBuffer);

		rawrbox::RENDERER->device()->CreateBuffer(BuffPixelDesc, nullptr, &this->_buffer);

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({{this->_buffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ---------------
	}

	void PostProcessPlugin::upload() {
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Post process plugin requires a main camera!");
		if (this->_postProcesses.empty()) return;

		// SWAP TEXTURE ----
		const auto* cameraTarget = rawrbox::MAIN_CAMERA->getRenderTarget();

		this->_swapTexture = std::make_unique<rawrbox::TextureRender>(cameraTarget->getSize(), false);
		this->_swapTexture->setName("PostProcess::Swap");
		this->_swapTexture->upload(cameraTarget->getTexture()->GetDesc().Format);
		// ----------

		for (const auto& postProcess : this->_postProcesses) {
			postProcess->init();
		}
	}

	void PostProcessPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
	}

	void PostProcessPlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "PostProcessConstants")->Set(this->_buffer);
	}

	void PostProcessPlugin::postRender(const rawrbox::CameraBase& camera) {
		if (&camera != rawrbox::MAIN_CAMERA) return; // Only apply on the main camera
		if (std::ranges::none_of(this->_postProcesses, [](const auto& process) { return process->isEnabled(); })) return;

		auto* context = rawrbox::RENDERER->context();
		auto* renderTarget = camera.getRenderTarget();
		auto* depth = renderTarget->getDepthHandle();

		if (renderTarget->getSize() != this->_swapTexture->getSize()) RAWRBOX_CRITICAL("Main camera render target size changed, post process swap texture no longer matches!");

		// DEPTH ---
		if (depth != nullptr) rawrbox::BarrierUtils::barrier({{depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		// ----------

		// Apply post-processing ---
		std::array<rawrbox::TextureRender*, 2> targets = {
			renderTarget,
			this->_swapTexture.get()
		};

		size_t read = 0;
		for (const auto& process : this->_postProcesses) {
			if (!process->isEnabled()) continue;

			process->applyEffect(*targets[read], *targets[1 - read], renderTarget->getDepthTextureID());
			read = 1 - read;
		}
		// ----------

		// COPY BACK ---
		if (read == 1) {
			auto* swapTex = this->_swapTexture->getTexture();
			auto* cameraTex = renderTarget->getTexture();

			rawrbox::BarrierUtils::barrier({
			    {swapTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_SOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
			    {cameraTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_COPY_DEST, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
			});

			Diligent::CopyTextureAttribs copy(swapTex, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY, cameraTex, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
			context->CopyTexture(copy);

			rawrbox::BarrierUtils::barrier({
			    {swapTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
			    {cameraTex, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
			});
		}
		// ----------

		// RESTORE ---
		auto* pRTV = rawrbox::RENDERER->swapChain()->GetCurrentBackBufferRTV();
		context->SetRenderTargets(1, &pRTV, rawrbox::RENDERER->swapChain()->GetDepthBufferDSV(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		if (depth != nullptr) rawrbox::BarrierUtils::barrier({{depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_READ, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});
		// ----------
	}

	// Post utils ----
	void PostProcessPlugin::remove(size_t indx) {
		if (this->_postProcesses.empty() || indx >= this->_postProcesses.size()) RAWRBOX_CRITICAL("Failed to remove {}!", indx);
		this->_postProcesses.erase(this->_postProcesses.begin() + indx);
	}

	rawrbox::PostProcessBase* PostProcessPlugin::get(size_t indx) const {
		if (indx >= this->_postProcesses.size()) RAWRBOX_CRITICAL("Failed to get {}!", indx);
		return this->_postProcesses[indx].get();
	}

	Diligent::IBuffer* PostProcessPlugin::getBuffer() const { return this->_buffer; }
	size_t PostProcessPlugin::count() const { return this->_postProcesses.size(); }

	std::string PostProcessPlugin::getID() { return "PostProcess"; }
	// ----
} // namespace rawrbox