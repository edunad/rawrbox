
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/render/bindless.hpp>
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/lights/manager.hpp>
#include <rawrbox/render/plugins/clustered.hpp>

namespace rawrbox {
	uint32_t ClusteredPlugin::CLUSTERS_X = 0;
	uint32_t ClusteredPlugin::CLUSTERS_Y = 0;
	uint32_t ClusteredPlugin::GROUP_SIZE = 0;

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

	const std::string ClusteredPlugin::getID() const { return "Clustered"; }

	void ClusteredPlugin::initialize(const rawrbox::Vector2i& renderSize) {
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.x, CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.y, CLUSTER_TEXTEL_SIZE);

		GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;

		if constexpr (CLUSTERS_Z % CLUSTERS_Z_THREADS != 0) throw this->_logger->error("Number of cluster depth slices must be divisible by thread count z-dimension");
		if constexpr (MAX_LIGHTS_PER_CLUSTER % 32 != 0) throw this->_logger->error("MAX_LIGHTS_PER_CLUSTER must be divisible by 32");

		// Setup dispatch ---
		this->_dispatch.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
		this->_dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
		this->_dispatch.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Z, CLUSTERS_Z_THREADS);
		// ----------

		// Initialize light engine
		rawrbox::LIGHTS::init();
		rawrbox::DECALS::init();
		// -----------------------

		this->buildBuffers();
	}

	void ClusteredPlugin::resize(const rawrbox::Vector2i& renderSize) {
		if (renderSize.x <= 0 || renderSize.y <= 0) return; // Minimized

		// Re-calculate clusters ----
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.x, CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.y, CLUSTER_TEXTEL_SIZE);

		GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;

		// Re-setup dispatch ---
		this->_dispatch.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
		this->_dispatch.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
		// ----------

		// --------------------------
		this->_oldProj = {}; // Re-build clusters
	}

	void ClusteredPlugin::upload() {
		this->buildPipelines();
	}

	void ClusteredPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig, bool compute) {
		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Clusters", 1, compute ? Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV : Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid", 1, compute ? Diligent::SHADER_RESOURCE_TYPE_BUFFER_UAV : Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);

		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);

		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Decals", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "DecalsConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
	}

	void ClusteredPlugin::bind(Diligent::IPipelineResourceSignature& sig, bool compute) {
		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Clusters")->Set(this->getClustersBuffer(!compute));
		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid")->Set(this->getDataGridBuffer(!compute));

		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Lights")->Set(rawrbox::LIGHTS::getBuffer());
		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);

		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "Decals")->Set(rawrbox::DECALS::getBuffer());
		sig.GetStaticVariableByName(compute ? Diligent::SHADER_TYPE_COMPUTE : Diligent::SHADER_TYPE_PIXEL, "DecalsConstants")->Set(rawrbox::DECALS::uniforms);
	}

	void ClusteredPlugin::preRender() {
		auto* renderer = rawrbox::RENDERER;
		auto* camera = renderer->camera();
		auto* context = renderer->context();

		if (renderer == nullptr) throw this->_logger->error("Renderer not initialized!");
		if (camera == nullptr) throw this->_logger->error("Camera not initialized!");

		if (this->_clusterBuildingComputeProgram == nullptr || this->_cullingComputeProgram == nullptr) throw this->_logger->error("Compute pipelines not initialized, did you call 'initialize'");

		// Setup uniforms
		rawrbox::LIGHTS::bindUniforms();
		rawrbox::DECALS::bindUniforms();
		// ------------

		// Barrier for writting -----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{this->_clusterBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS}, {this->_dataGridBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS}});
		// -----------

		// Commit compute signature --
		context->CommitShaderResources(rawrbox::BindlessManager::computeSignatureBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_VERIFY);
		// -----------------------

		// Rebuild clusters ---
		auto proj = camera->getProjMtx();
		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			context->SetPipelineState(this->_clusterBuildingComputeProgram);
			context->DispatchCompute(this->_dispatch);
		}
		// ------

		// Perform light / decal culling
		// uint32_t ClearValue = 0;
		// context->ClearUAVUint(this->_dataGridBuffer, &ClearValue);
		context->SetPipelineState(this->_cullingComputeProgram);
		context->DispatchCompute(this->_dispatch);
		// ----------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{this->_clusterBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE}, {this->_dataGridBuffer, Diligent::RESOURCE_STATE_SHADER_RESOURCE}});
		// -----------
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
			BuffDesc.Size = BuffDesc.ElementByteStride * GROUP_SIZE;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_clusterBuffer);

			this->_clusterBufferWrite = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_clusterBufferRead = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// --------------

		// Data grid ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(std::array<uint32_t, 2>);
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::ClusterDataGrid";
			BuffDesc.Size = BuffDesc.ElementByteStride * (rawrbox::MAX_LIGHTS_PER_CLUSTER / rawrbox::CLUSTERS_Z * GROUP_SIZE);
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			device->CreateBuffer(BuffDesc, nullptr, &this->_dataGridBuffer);

			this->_dataGridBufferWrite = this->_dataGridBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_dataGridBufferRead = this->_dataGridBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// ------------------

		// BARRIER -----
		rawrbox::BarrierUtils::barrier<Diligent::IBuffer>({{this->_clusterBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS}, {this->_dataGridBuffer, Diligent::RESOURCE_STATE_UNORDERED_ACCESS}});
		// -----------
	}

	void ClusteredPlugin::buildPipelines() {
		if (rawrbox::MAIN_CAMERA == nullptr) throw _logger->error("Clustered plugin requires at least one camera!");

		rawrbox::PipeComputeSettings settings;
		settings.macros = this->getClusterMacros();
		settings.signature = rawrbox::BindlessManager::computeSignature;

		// BUILDING -----
		settings.pCS = "cluster_build.csh";
		this->_clusterBuildingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Build", settings);
		// ---------

		// CULLING -----
		settings.pCS = "cluster_cull.csh";
		this->_cullingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Cull", settings);
		//  ----
	}

	// UTILS ----
	Diligent::ShaderMacroHelper ClusteredPlugin::getClusterMacros() {
		Diligent::ShaderMacroHelper macro;

		macro.AddShaderMacro("THREAD_GROUP_SIZE", rawrbox::THREAD_GROUP_SIZE);
		// macro.AddShaderMacro("GROUP_SIZE", GROUP_SIZE);
		macro.AddShaderMacro("CLUSTERS_X_THREADS", rawrbox::CLUSTERS_X_THREADS);
		macro.AddShaderMacro("CLUSTERS_Y_THREADS", rawrbox::CLUSTERS_Y_THREADS);
		macro.AddShaderMacro("CLUSTERS_Z_THREADS", rawrbox::CLUSTERS_Z_THREADS);
		macro.AddShaderMacro("CLUSTERS_X", CLUSTERS_X);
		macro.AddShaderMacro("CLUSTERS_Y", CLUSTERS_Y);
		macro.AddShaderMacro("CLUSTERS_Z", rawrbox::CLUSTERS_Z);
		macro.AddShaderMacro("MAX_LIGHTS_PER_CLUSTER", rawrbox::MAX_LIGHTS_PER_CLUSTER);
		macro.AddShaderMacro("CLUSTER_TEXTEL_SIZE", rawrbox::CLUSTER_TEXTEL_SIZE);
		macro.AddShaderMacro("CLUSTERED_NUM_BUCKETS", rawrbox::CLUSTERED_NUM_BUCKETS);

		macro.AddShaderMacro("CLUSTER_PLUGIN", true);
		return macro;
	}

	Diligent::IBufferView* ClusteredPlugin::getClustersBuffer(bool readOnly) { return readOnly ? this->_clusterBufferRead : this->_clusterBufferWrite; }
	Diligent::IBufferView* ClusteredPlugin::getDataGridBuffer(bool readOnly) { return readOnly ? this->_dataGridBufferRead : this->_dataGridBufferWrite; }
	// ----------

} // namespace rawrbox
