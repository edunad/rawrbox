
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {
	uint32_t ClusteredPlugin::CLUSTERS_X = 0;
	uint32_t ClusteredPlugin::CLUSTERS_Y = 0;
	uint32_t ClusteredPlugin::CLUSTERS_GROUP_SIZE = 0;

	ClusteredPlugin::~ClusteredPlugin() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system
		rawrbox::DECALS::shutdown(); // Shutdown decal system

		RAWRBOX_DESTROY(this->_clusterBuffer);
		RAWRBOX_DESTROY(this->_clusterBufferWrite);
		RAWRBOX_DESTROY(this->_clusterBufferRead);

		RAWRBOX_DESTROY(this->_dataGridBuffer);
		RAWRBOX_DESTROY(this->_dataGridBufferWrite);
		RAWRBOX_DESTROY(this->_dataGridBufferRead);
	}

	void ClusteredPlugin::initialize(const rawrbox::Vector2u& renderSize) {
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.x, RB_RENDER_CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.y, RB_RENDER_CLUSTER_TEXTEL_SIZE);

		CLUSTERS_GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * RB_RENDER_CLUSTERS_Z;

		if constexpr (RB_RENDER_CLUSTERS_Z % RB_RENDER_CLUSTERS_Z_THREADS != 0) RAWRBOX_CRITICAL("Number of cluster depth slices must be divisible by thread count z-dimension");
		if constexpr (RB_RENDER_MAX_DATA_PER_CLUSTER % 32 != 0) RAWRBOX_CRITICAL("MAX_DATA_PER_CLUSTER must be divisible by 32");

		// Setup dispatch ---
		this->_dispatch.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, RB_RENDER_CLUSTERS_X_THREADS);
		this->_dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, RB_RENDER_CLUSTERS_Y_THREADS);
		this->_dispatch.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(RB_RENDER_CLUSTERS_Z, RB_RENDER_CLUSTERS_Z_THREADS);
		// ----------

		// Initialize light engine
		rawrbox::LIGHTS::init();
		rawrbox::DECALS::init();
		// -----------------------

		this->buildBuffers();
		this->buildSignatures();
	}

	void ClusteredPlugin::resize(const rawrbox::Vector2u& renderSize) {
		if (renderSize.x <= 0 || renderSize.y <= 0) return; // Minimized

		// Re-calculate clusters ----
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.x, RB_RENDER_CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.y, RB_RENDER_CLUSTER_TEXTEL_SIZE);
		CLUSTERS_GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * RB_RENDER_CLUSTERS_Z;
		// -------------------------

		// Re-setup dispatch ---
		this->_dispatch.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, RB_RENDER_CLUSTERS_X_THREADS);
		this->_dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, RB_RENDER_CLUSTERS_Y_THREADS);
		// ----------

		// Re-build clusters ---
		this->_oldProj = {};
		// --------------------------
	}

	void ClusteredPlugin::upload() {
		if (this->_signature == nullptr) RAWRBOX_CRITICAL("Signature not initialized, did you call 'initialize'?");
		if (rawrbox::MAIN_CAMERA == nullptr) RAWRBOX_CRITICAL("Main camera not initialized");

		// Compute bind ---
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Camera")->Set(rawrbox::CameraBase::uniforms);
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "SCamera")->Set(rawrbox::CameraBase::staticUniforms);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Clusters")->Set(this->getClustersBuffer(false));
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "ClusterDataGrid")->Set(this->getDataGridBuffer(false));

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Lights")->Set(rawrbox::LIGHTS::getBuffer());
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);

		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "Decals")->Set(rawrbox::DECALS::getBuffer());
		this->_signature->GetStaticVariableByName(Diligent::SHADER_TYPE_COMPUTE, "DecalsConstants")->Set(rawrbox::DECALS::uniforms);

		this->_signature->CreateShaderResourceBinding(&this->_signatureBind, true);
		// ----------------

		this->buildPipelines();
	}

	void ClusteredPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Clusters", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Decals", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "DecalsConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS);
	}

	void ClusteredPlugin::bindStatic(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Clusters")->Set(this->getClustersBuffer(true));
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid")->Set(this->getDataGridBuffer(true));

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

		if (this->_clusterBuildingComputeProgram == nullptr || this->_cullingComputeProgram == nullptr || this->_cullingResetProgram == nullptr) RAWRBOX_CRITICAL("Compute pipelines not initialized, did you call 'initialize'");

		// Update light & decals
		rawrbox::LIGHTS::update();
		rawrbox::DECALS::update();
		// ------------

		// Barrier for writting -----
		rawrbox::BarrierUtils::barrier({
		    {this->_clusterBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_dataGridBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		//  -----------

		// Commit compute signature --
		context->CommitShaderResources(this->_signatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		// -----------------------

		// Rebuild clusters ---
		auto proj = camera.getProjMtx();
		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			context->SetPipelineState(this->_clusterBuildingComputeProgram);
			context->DispatchCompute(this->_dispatch);
		}
		// ------

		// Reset clusters
		// TODO: REPLACE WITH
		// uint32_t ClearValue = 0;
		// context->ClearUAVUint(this->_dataGridBuffer, &ClearValue);
		context->SetPipelineState(this->_cullingResetProgram);
		context->DispatchCompute(this->_dispatch);
		//   --------------

		// Perform light / decal culling
		context->SetPipelineState(this->_cullingComputeProgram);
		context->DispatchCompute(this->_dispatch);
		// ----------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_clusterBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_dataGridBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		});
		//  -----------
	}

	void ClusteredPlugin::buildBuffers() {
		auto* device = rawrbox::RENDERER->device();

		// Clusters  --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::ClusterAABB);
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::Cluster";
			BuffDesc.Size = BuffDesc.ElementByteStride * CLUSTERS_GROUP_SIZE;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_clusterBuffer);

			this->_clusterBufferWrite = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_clusterBufferRead = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// --------------

		// Data grid ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(std::array<uint32_t, 4>);
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::ClusterDataGrid";
			BuffDesc.Size = BuffDesc.ElementByteStride * (RB_RENDER_MAX_DATA_PER_CLUSTER / RB_RENDER_CLUSTERS_Z * CLUSTERS_GROUP_SIZE);
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_dataGridBuffer);

			this->_dataGridBufferWrite = this->_dataGridBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_dataGridBufferRead = this->_dataGridBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// ------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier({
		    {this->_clusterBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
		    {this->_dataGridBuffer, Diligent::RESOURCE_STATE_UNKNOWN, Diligent::RESOURCE_STATE_SHADER_RESOURCE, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE},
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

		    // CLUSTERS ---
		    {Diligent::SHADER_TYPE_COMPUTE, "Clusters", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "ClusterDataGrid", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // ------------

		    // LIGHT -----
		    {Diligent::SHADER_TYPE_COMPUTE, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    {Diligent::SHADER_TYPE_COMPUTE, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
		    // -----------

		    // DECAL -----
		    {Diligent::SHADER_TYPE_COMPUTE, "Decals", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC, Diligent::PIPELINE_RESOURCE_FLAG_NO_DYNAMIC_BUFFERS},
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

		// BUILDING -----
		settings.pCS = "cluster_build.csh";
		this->_clusterBuildingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Build", settings);
		// ---------

		// RESET -----
		settings.pCS = "cluster_reset.csh";
		this->_cullingResetProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Reset", settings);
		//  ----

		// CULLING -----
		settings.pCS = "cluster_cull.csh";
		this->_cullingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Cull", settings);
		//  ----
	}

	// UTILS ----
	Diligent::ShaderMacroHelper ClusteredPlugin::getClusterMacros() {
		Diligent::ShaderMacroHelper macro;

		macro.AddShaderMacro("THREAD_GROUP_SIZE", RB_RENDER_THREAD_GROUP_SIZE);
		macro.AddShaderMacro("CLUSTERS_X_THREADS", RB_RENDER_CLUSTERS_X_THREADS);
		macro.AddShaderMacro("CLUSTERS_Y_THREADS", RB_RENDER_CLUSTERS_Y_THREADS);
		macro.AddShaderMacro("CLUSTERS_Z_THREADS", RB_RENDER_CLUSTERS_Z_THREADS);
		macro.AddShaderMacro("CLUSTERS_X", CLUSTERS_X);
		macro.AddShaderMacro("CLUSTERS_Y", CLUSTERS_Y);
		macro.AddShaderMacro("CLUSTERS_Z", RB_RENDER_CLUSTERS_Z);
		macro.AddShaderMacro("MAX_DATA_PER_CLUSTER", RB_RENDER_MAX_DATA_PER_CLUSTER);
		macro.AddShaderMacro("CLUSTER_TEXTEL_SIZE", RB_RENDER_CLUSTER_TEXTEL_SIZE);
		macro.AddShaderMacro("CLUSTERED_NUM_BUCKETS", RB_RENDER_CLUSTERED_NUM_BUCKETS);

		macro.AddShaderMacro("CLUSTER_PLUGIN", true);
		return macro;
	}

	Diligent::IBufferView* ClusteredPlugin::getClustersBuffer(bool readOnly) { return readOnly ? this->_clusterBufferRead : this->_clusterBufferWrite; }
	Diligent::IBufferView* ClusteredPlugin::getDataGridBuffer(bool readOnly) { return readOnly ? this->_dataGridBufferRead : this->_dataGridBufferWrite; }
	// ----------

	std::string ClusteredPlugin::getID() { return "Clustered"; }

} // namespace rawrbox
