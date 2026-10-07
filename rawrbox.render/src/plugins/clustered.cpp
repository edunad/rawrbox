#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

#include <algorithm>

namespace rawrbox {
	ClusteredPlugin::~ClusteredPlugin() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system
		rawrbox::DECALS::shutdown(); // Shutdown decal system

		// Cleanup ----
		RAWRBOX_DESTROY(this->_lightTiles);
		RAWRBOX_DESTROY(this->_lightTilesWrite);
		RAWRBOX_DESTROY(this->_lightTilesRead);

		RAWRBOX_DESTROY(this->_decalTiles);
		RAWRBOX_DESTROY(this->_decalTilesWrite);
		RAWRBOX_DESTROY(this->_decalTilesRead);
		// -------------
	}

	void ClusteredPlugin::initialize(const rawrbox::Vector2u& /*renderSize*/) {
		if constexpr (RB_RENDER_MAX_LIGHTS % BIN_BITS != 0 || RB_RENDER_MAX_LIGHTS > 65535)
			RAWRBOX_CRITICAL("RB_RENDER_MAX_LIGHTS must be divisible by {}, and fit in 16 bits", BIN_BITS);

		if constexpr (RB_RENDER_MAX_DECALS % BIN_BITS != 0 || RB_RENDER_MAX_DECALS > 65535)
			RAWRBOX_CRITICAL("RB_RENDER_MAX_DECALS must be divisible by {}, and fit in 16 bits", BIN_BITS);

		// Init
		rawrbox::LIGHTS::init();
		rawrbox::DECALS::init();
		// -----------------------

		this->buildBuffers();
		this->buildSignatures();
	}

	void ClusteredPlugin::upload() {
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Signature not initialized, did you call 'initialize'?");
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Main camera not initialized");

		// Compute bind ---
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Camera")->Set(rawrbox::CameraBase::uniforms);
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightTiles")->Set(this->getLightTilesBuffer(false));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "DecalTiles")->Set(this->getDecalTilesBuffer(false));

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightBounds")->Set(rawrbox::LIGHTS::getBoundsBuffer());
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "DecalBounds")->Set(rawrbox::DECALS::getBoundsBuffer());
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "DecalsConstants")->Set(rawrbox::DECALS::uniforms);

		this->_signature->CreateShaderResourceBinding(&this->_signatureBind, true);
		// ----------------

		this->buildPipelines();
	}

	void ClusteredPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "LightTiles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "LightZBins", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "DecalTiles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "DecalZBins", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Decals", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "DecalsConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
	}

	void ClusteredPlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "LightTiles")->Set(this->getLightTilesBuffer(true));
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "LightZBins")->Set(rawrbox::LIGHTS::getZBinBuffer());

		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "DecalTiles")->Set(this->getDecalTilesBuffer(true));
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "DecalZBins")->Set(rawrbox::DECALS::getZBinBuffer());

		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Lights")->Set(rawrbox::LIGHTS::getBuffer());
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);

		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Decals")->Set(rawrbox::DECALS::getBuffer());
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "DecalsConstants")->Set(rawrbox::DECALS::uniforms);
	}

	void ClusteredPlugin::preRender(const rawrbox::CameraBase& camera) {
		auto* renderer = rawrbox::RENDERER;
		if (renderer == nullptr) RAWRBOX_CRITICAL("Renderer not initialized!");

		auto* context = renderer->context();
		if (context == nullptr) RAWRBOX_CRITICAL("Context not initialized!");

		if (this->_tileCullProgram == nullptr) RAWRBOX_CRITICAL("Compute pipelines not initialized, did you call 'upload'");

		// Sort ---
		const bool binned = this->isBinned(camera);

		const bool lightsRebuilt = rawrbox::LIGHTS::update(camera, binned);
		const bool decalsRebuilt = rawrbox::DECALS::update(camera, binned);
		if (!lightsRebuilt && !decalsRebuilt) return; // Tiles only depend on the bins, nothing to cull
		// ------------

		const uint32_t lightBins = rawrbox::MathUtils::divideRound<uint32_t>(static_cast<uint32_t>(rawrbox::LIGHTS::active()), BIN_BITS);
		const uint32_t decalBins = rawrbox::MathUtils::divideRound<uint32_t>(static_cast<uint32_t>(std::min<size_t>(rawrbox::DECALS::count(), RB_RENDER_MAX_DECALS)), BIN_BITS);

		const uint32_t maxBins = std::max(lightBins, decalBins);
		if (maxBins == 0) return;

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_lightTiles, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_decalTiles, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------

		// Bitmasks --
		context->CommitShaderResources(this->_signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		context->SetPipelineState(this->_tileCullProgram);

		Diligent::DispatchComputeAttribs dispatch;

		dispatch.ThreadGroupCountX = TILES;
		dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(maxBins, RB_RENDER_TILE_THREADS);
		dispatch.ThreadGroupCountZ = 2;

		context->DispatchCompute(dispatch);
		// ----------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_lightTiles, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_decalTiles, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------
	}

	void ClusteredPlugin::buildBuffers() {
		auto* device = rawrbox::RENDERER->device();

		// Light ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::LightTiles";
			BuffDesc.Size = BuffDesc.ElementByteStride * TILES * LIGHT_BINS;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_lightTiles);

			this->_lightTilesWrite = this->_lightTiles->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_lightTilesRead = this->_lightTiles->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// --------------

		// Decal ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::DecalTiles";
			BuffDesc.Size = BuffDesc.ElementByteStride * TILES * DECAL_BINS;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_decalTiles);

			this->_decalTilesWrite = this->_decalTiles->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_decalTilesRead = this->_decalTiles->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// ------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_lightTiles, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_decalTiles, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		// -----------
	}

	void ClusteredPlugin::buildSignatures() {
		if (this->_signature != nullptr || this->_signatureBind != nullptr) RAWRBOX_CRITICAL("Signatures already bound!");

		std::vector<Diligent::PipelineResourceDesc> resources = {
		    // CAMERA ------
		    {Diligent::SHADER_TYPE_COMPUTE, "Camera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "SCamera", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // --------------

		    // TILES ---
		    {Diligent::SHADER_TYPE_COMPUTE, "LightTiles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "DecalTiles", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // ------------

		    // LIGHT -----
		    {Diligent::SHADER_TYPE_COMPUTE, "LightBounds", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // -----------

		    // DECAL -----
		    {Diligent::SHADER_TYPE_COMPUTE, "DecalBounds", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "DecalsConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // -----------
		};

		// Compute signature ---
		Diligent::PipelineResourceSignatureDesc PRSDesc;
		PRSDesc.Name = "RawrBox::SIGNATURE::Clustered";
		PRSDesc.BindingIndex = 0;

		PRSDesc.ImmutableSamplers = nullptr;
		PRSDesc.NumImmutableSamplers = 0;

		PRSDesc.Resources = resources.data();
		PRSDesc.NumResources = static_cast<uint8_t>(resources.size());

		rawrbox::RENDERER->device()->CreatePipelineResourceSignature(PRSDesc, &this->_signature);
		// ----------------------
	}

	void ClusteredPlugin::buildPipelines() {
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Signature not initialized, did you call 'initialize'");

		rawrbox::PipeComputeSettings settings;
		settings.macros = this->getClusterMacros();
		settings.signatures = {this->_signature};

		// TILE CULLING -----
		settings.pCS = "tile_cull.csh";
		this->_tileCullProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::TileCull", settings);
		// ----
	}

	// CAMERAS ----
	void ClusteredPlugin::addCamera(const rawrbox::CameraBase& camera) {
		if (this->isBinned(camera)) return;
		this->_cameras.push_back(&camera);
	}

	void ClusteredPlugin::removeCamera(const rawrbox::CameraBase& camera) {
		std::erase(this->_cameras, &camera);
	}

	bool ClusteredPlugin::isBinned(const rawrbox::CameraBase& camera) const {
		if (&camera == rawrbox::MAIN_CAMERA) return true;
		return std::ranges::find(this->_cameras, &camera) != this->_cameras.end();
	}
	// ----------

	// UTILS ----
	Diligent::ShaderMacroHelper ClusteredPlugin::getClusterMacros() {
		Diligent::ShaderMacroHelper macro;

		macro.AddShaderMacro("TILES_X", RB_RENDER_TILES_X);
		macro.AddShaderMacro("TILES_Y", RB_RENDER_TILES_Y);
		macro.AddShaderMacro("TILE_THREADS", RB_RENDER_TILE_THREADS);

		macro.AddShaderMacro("ZBINS", RB_RENDER_ZBINS);
		macro.AddShaderMacro("BIN_BITS", BIN_BITS);

		macro.AddShaderMacro("LIGHT_BINS", LIGHT_BINS);
		macro.AddShaderMacro("DECAL_BINS", DECAL_BINS);

		macro.AddShaderMacro("CLUSTER_PLUGIN", true);
		return macro;
	}

	Diligent::IBufferView* ClusteredPlugin::getLightTilesBuffer(bool readOnly) {
		return readOnly ? this->_lightTilesRead : this->_lightTilesWrite;
	}

	Diligent::IBufferView* ClusteredPlugin::getDecalTilesBuffer(bool readOnly) {
		return readOnly ? this->_decalTilesRead : this->_decalTilesWrite;
	}
	// ----------

	std::string ClusteredPlugin::getID() { return "Clustered"; }

} // namespace rawrbox
