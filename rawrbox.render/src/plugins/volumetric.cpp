#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/plugins/clustered.hpp>
#include <rawrbox/render/plugins/shadows.hpp>
#include <rawrbox/render/plugins/volumetric.hpp>
#include <rawrbox/render/utils/barrier.hpp>
#include <rawrbox/render/utils/pipeline.hpp>

#include <algorithm>
#include <cmath>

namespace rawrbox {
	VolumetricPlugin::VolumetricPlugin(const rawrbox::VolumetricSettings& settings) : _settings(settings) {
		this->_data.reserve(RB_RENDER_MAX_LIGHTS);
	}

	VolumetricPlugin::~VolumetricPlugin() {
		this->_scatter.reset();
		for (auto& history : this->_history) {
			history.reset();
		}

		RAWRBOX_DESTROY(this->_constants);
		RAWRBOX_DESTROY(this->_lightBuffer);
	}

	// LIGHTS ---
	void VolumetricPlugin::add(const rawrbox::LightBase& light, const rawrbox::VolumetricLightSettings& settings) {
		if (light.getType() == rawrbox::LightType::DIRECTIONAL) RAWRBOX_CRITICAL("Directional volumetric lights are not supported");

		std::erase_if(this->_lights, [](const auto& entry) { return entry.light.expired(); });

		const auto existing = std::ranges::find_if(this->_lights, [&light](const auto& entry) { return entry.light.lock().get() == &light; });
		if (existing != this->_lights.end()) {
			existing->settings = settings;
			return;
		}

		const auto& lights = rawrbox::LIGHTS::getLights();
		const auto fnd = std::ranges::find_if(lights, [&light](const auto& entry) { return entry.get() == &light; });
		if (fnd == lights.end()) RAWRBOX_CRITICAL("Light not found, did you call 'LIGHTS::add'?");

		this->_lights.push_back({.light = *fnd, .settings = settings});
	}

	void VolumetricPlugin::remove(const rawrbox::LightBase& light) {
		std::erase_if(this->_lights, [&light](const auto& entry) { return entry.light.expired() || entry.light.lock().get() == &light; });
	}
	// -----------

	void VolumetricPlugin::initialize(const rawrbox::Vector2u& /*size*/) {
		if (this->_settings.divisions == 0 || this->_settings.steps == 0 || this->_settings.levels == 0) RAWRBOX_CRITICAL("Invalid VolumetricSettings! divisor, steps & levels must be above 0");
		if (this->_settings.blend <= 0.F || this->_settings.blend > 1.F) RAWRBOX_CRITICAL("Invalid VolumetricSettings! blend must be between 0 and 1");
		if (this->_constants != nullptr) RAWRBOX_CRITICAL("Plugin already initialized!");

		auto* device = rawrbox::RENDERER->device();
		if (device == nullptr) RAWRBOX_CRITICAL("Device is not set!");

		if ((device->GetTextureFormatInfoExt(Diligent::TEX_FORMAT_R11G11B10_FLOAT).BindFlags & Diligent::BIND_RENDER_TARGET) == 0)
			RAWRBOX_CRITICAL("R11G11B10 render targets are not supported on this device, required by the volumetric plugin");

		// Constants ---
		{
			Diligent::BufferDesc desc;
			desc.Name = "RawrBox::Volumetric::Constants";
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			desc.Size = sizeof(rawrbox::VolumetricConstants);

			device->CreateBuffer(desc, nullptr, &this->_constants);
			if (this->_constants == nullptr) RAWRBOX_CRITICAL("Failed to create buffer '{}'", desc.Name);
		}
		// -------------

		// Lights ---
		{
			Diligent::BufferDesc desc;
			desc.Name = "RawrBox::Volumetric::Lights";
			desc.Usage = Diligent::USAGE_DEFAULT;
			desc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			desc.BindFlags = Diligent::BIND_SHADER_RESOURCE;
			desc.ElementByteStride = sizeof(rawrbox::VolumetricLightData);
			desc.Size = static_cast<uint64_t>(desc.ElementByteStride) * RB_RENDER_MAX_LIGHTS; // Volumetric lights are a subset of the scene lights

			device->CreateBuffer(desc, nullptr, &this->_lightBuffer);
			if (this->_lightBuffer == nullptr) RAWRBOX_CRITICAL("Failed to create buffer '{}'", desc.Name);
		}
		// -------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_constants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_lightBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------
	}

	void VolumetricPlugin::upload() {
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Volumetric plugin requires a main camera!");
		if (this->_scatter != nullptr) RAWRBOX_CRITICAL("Plugin already uploaded!");

		auto* cluster = rawrbox::RENDERER->getPlugin<rawrbox::ClusteredPlugin>("Clustered");
		if (cluster == nullptr) RAWRBOX_CRITICAL("Volumetric plugin requires the `Clustered` renderer plugin");

		const auto divisor = this->_settings.divisions;
		const auto* cameraTarget = rawrbox::MAIN_CAMERA->getRenderTarget();
		const auto& size = cameraTarget->getSize();

		// SCATTER TEXTURES ----
		const rawrbox::Vector2u scatterSize = {rawrbox::MathUtils::divideRound(size.x, divisor), rawrbox::MathUtils::divideRound(size.y, divisor)};

		this->_scatter = std::make_unique<rawrbox::TextureRender>(scatterSize, false);
		this->_scatter->setName("Volumetric::Scatter");
		this->_scatter->upload(Diligent::TEX_FORMAT_R11G11B10_FLOAT);

		for (auto& history : this->_history) {
			history = std::make_unique<rawrbox::TextureRender>(scatterSize, false);
			history->setName("Volumetric::History");
			history->upload(Diligent::TEX_FORMAT_R11G11B10_FLOAT);
		}
		// ----------

		// PIPELINES ---
		rawrbox::PipeSettings settings;
		settings.pVS = "quad.vsh";
		settings.topology = Diligent::PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		settings.cull = Diligent::CULL_MODE_NONE;
		settings.depth = Diligent::COMPARISON_FUNC_UNKNOWN;
		settings.depthWrite = false;
		settings.depthFormat = false;
		settings.blending = {.src = Diligent::BLEND_FACTOR_UNDEFINED, .dest = Diligent::BLEND_FACTOR_UNDEFINED};
		settings.macros = cluster->getClusterMacros();
		settings.signatures = {rawrbox::BindlessManager::signature};

		auto* shadows = rawrbox::RENDERER->getPlugin<rawrbox::ShadowPlugin>("Shadows");
		if (shadows != nullptr) settings.macros += shadows->getShadowMacros();

		settings.macros.AddShaderMacro("VOLUMETRIC_DIVISIONS", divisor);

		settings.macros.AddShaderMacro("VOLUMETRIC_STEPS", this->_settings.steps);
		settings.macros.AddShaderMacro("VOLUMETRIC_LEVELS", this->_settings.levels);
		settings.macros.AddShaderMacro("VOLUMETRIC_BLEND", this->_settings.blend);

		settings.pPS = "volumetric.psh";
		settings.psEntry = "March";
		settings.renderTargetFormat = Diligent::TEX_FORMAT_R11G11B10_FLOAT;
		this->_marchProgram = rawrbox::PipelineUtils::createPipeline("Volumetric::March", settings);

		settings.psEntry = "Resolve";
		this->_resolveProgram = rawrbox::PipelineUtils::createPipeline("Volumetric::Resolve", settings);

		settings.psEntry = "Composite";
		settings.renderTargetFormat = cameraTarget->getTexture()->GetDesc().Format;
		settings.blending = {.src = Diligent::BLEND_FACTOR_ONE, .dest = Diligent::BLEND_FACTOR_ONE};
		this->_compositeProgram = rawrbox::PipelineUtils::createPipeline("Volumetric::Composite", settings);
		// ----------
	}

	void VolumetricPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "VolumetricConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "VolumetricLights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
	}

	void VolumetricPlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "VolumetricConstants")->Set(this->_constants);
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "VolumetricLights")->Set(this->_lightBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
	}

	void VolumetricPlugin::postRender(const rawrbox::CameraBase& camera) {
		if (!rawrbox::LIGHTS::isEnabled()) return;   // Full bright
		if (&camera != rawrbox::MAIN_CAMERA) return; // Skip non-main cameras

		if (this->_marchProgram == nullptr || this->_resolveProgram == nullptr || this->_compositeProgram == nullptr) RAWRBOX_CRITICAL("Pipelines not initialized, did you call 'upload'");

		const auto viewProj = camera.getProjMtx() * camera.getViewMtx();

		// CULLED LIGHTS ---
		this->_data.clear();

		for (const auto& entry : this->_lights) {
			auto light = entry.light.lock();

			if (light == nullptr || !light->isActive()) continue;
			if (this->_data.size() >= RB_RENDER_MAX_LIGHTS) break;

			auto data = rawrbox::LIGHTS::getVertexData(*light);

			// Spot ---
			float start = 0.F;
			float apex = 0.F;

			if (light->getType() == rawrbox::LightType::SPOT) {
				const float sinUmbra = std::sqrt(std::max(1.F - data.cosUmbra * data.cosUmbra, 1e-6F));
				start = entry.settings.width * 0.5F * data.cosUmbra / sinUmbra;
				apex = entry.settings.offset + start;
				data.direction = data.direction.normalized();
			}
			// -----

			// FRUSTUM ---
			const float reach = data.radius + 2.F * apex;
			if (!rawrbox::BBOX{data.position - reach, data.position + reach, rawrbox::Vector3f{reach * 2.F, reach * 2.F, reach * 2.F}}.inFrustum(viewProj)) continue;
			// -----

			this->_data.push_back({.light = data, .params = {entry.settings.density, apex, start, 0.F}});
		}

		if (this->_data.empty()) return;
		// -----------

		auto* context = rawrbox::RENDERER->context();
		if (context == nullptr) RAWRBOX_CRITICAL("Invalid renderer context");

		const auto* renderTarget = camera.getRenderTarget();
		auto* depth = renderTarget->getDepthHandle();

		// HISTORY ---
		const auto* previous = this->_history[1 - this->_historyIndex].get();
		auto* current = this->_history[this->_historyIndex].get();

		const bool historyValid = this->_prevFrame != 0 && rawrbox::FRAME == this->_prevFrame + 1;
		// -----------

		// UPLOAD ---
		rawrbox::VolumetricConstants constants = {};
		for (uint32_t i = 0; i < 4; i++) {
			constants.prevClip[i] = {this->_prevViewProj[i], this->_prevViewProj[4 + i], this->_prevViewProj[8 + i], this->_prevViewProj[12 + i]};
		}

		constants.settings = {
		    static_cast<uint32_t>(this->_data.size()),
		    renderTarget->getDepthTextureID() | (this->_scatter->getTextureID() << 16U),
		    (historyValid ? previous->getTextureID() : 0U) | (current->getTextureID() << 16U),
		    this->_jitterFrame++};

		context->UpdateBuffer(this->_constants, 0, sizeof(rawrbox::VolumetricConstants), &constants, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		context->UpdateBuffer(this->_lightBuffer, 0, this->_data.size() * sizeof(rawrbox::VolumetricLightData), this->_data.data(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		rawrbox::BarrierUtils::barrier({
		    {this->_constants, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_CONSTANT_BUFFER, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_lightBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------

		Diligent::DrawAttribs draw;
		draw.NumVertices = 4;

		// CUBE-MARCH ---
		this->_scatter->startRecord(false);

		context->SetPipelineState(this->_marchProgram);
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		context->Draw(draw);

		this->_scatter->stopRecord();
		// -----------

		// RESOLVE ---
		current->startRecord(false);

		context->SetPipelineState(this->_resolveProgram);
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		context->Draw(draw);

		current->stopRecord();

		this->_historyIndex = 1 - this->_historyIndex;
		this->_prevViewProj = viewProj;
		this->_prevFrame = rawrbox::FRAME;
		// -----------

		// COMPOSITE ---
		auto* color = camera.getColor(true);
		rawrbox::BarrierUtils::barrier({{color->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_RENDER_TARGET, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE}});

		context->SetRenderTargets(1, &color, nullptr, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->SetPipelineState(this->_compositeProgram);
		context->CommitShaderResources(rawrbox::BindlessManager::signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_NONE);
		context->Draw(draw);
		// -----------

		// RESTORE ---
		auto* backBuffer = rawrbox::RENDERER->swapChain()->GetCurrentBackBufferRTV();
		context->SetRenderTargets(1, &backBuffer, rawrbox::RENDERER->swapChain()->GetDepthBufferDSV(), Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);

		rawrbox::BarrierUtils::barrier({
		    {color->GetTexture(), Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {depth, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_DEPTH_READ, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------
	}

	short VolumetricPlugin::getPriority() const { return 50; }
	std::string VolumetricPlugin::getID() { return "Volumetric"; }
} // namespace rawrbox