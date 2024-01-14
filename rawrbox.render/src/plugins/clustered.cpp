
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

		auto check = CLUSTERS_Z % CLUSTERS_Z_THREADS != 0;
		if (check) throw this->_logger->error("Number of cluster depth slices must be divisible by thread count z-dimension");

		// Initialize light engine
		rawrbox::LIGHTS::init();
		rawrbox::DECALS::init();
		// -----------------------

		this->buildBuffers();
		this->buildPipelines();
	}

	void ClusteredPlugin::resize(const rawrbox::Vector2i& renderSize) {
		// Re-calculate clusters ----
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.x, CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(renderSize.y, CLUSTER_TEXTEL_SIZE);

		GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;
		// --------------------------

		this->_oldProj = {}; // Re-build clusters
	}

	void ClusteredPlugin::signatures(std::vector<Diligent::PipelineResourceDesc>& sig) {
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Lights", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "LightConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);

		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "Decals", 1, Diligent::SHADER_RESOURCE_TYPE_BUFFER_SRV, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
		sig.emplace_back(Diligent::SHADER_TYPE_PIXEL, "DecalsConstants", 1, Diligent::SHADER_RESOURCE_TYPE_CONSTANT_BUFFER, Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC);
	}

	void ClusteredPlugin::bind(Diligent::IPipelineResourceSignature& sig) {
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "ClusterDataGrid")->Set(this->getDataGridBuffer());

		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Lights")->Set(rawrbox::LIGHTS::getBuffer());
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "LightConstants")->Set(rawrbox::LIGHTS::uniforms);

		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "Decals")->Set(rawrbox::DECALS::getBuffer());
		sig.GetStaticVariableByName(Diligent::SHADER_TYPE_PIXEL, "DecalsConstants")->Set(rawrbox::DECALS::uniforms);
	}

	void ClusteredPlugin::preRender() {
		auto renderer = rawrbox::RENDERER;
		if (renderer == nullptr) throw this->_logger->error("Renderer not initialized!");
		if (this->_clusterBuildingComputeProgram == nullptr || this->_lightCullingComputeBind == nullptr) throw this->_logger->error("Compute pipelines not initialized, did you call 'initialize'");

		auto camera = renderer->camera();
		auto context = renderer->context();

		// Setup uniforms
		rawrbox::LIGHTS::bindUniforms();
		rawrbox::DECALS::bindUniforms();
		// ------------

		// Only rebuild cluster if view changed
		auto proj = camera->getProjMtx();
		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			// Rebuild clusters ---
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
			DispatchAttribs.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
			DispatchAttribs.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Z, CLUSTERS_Z_THREADS);

			rawrbox::BindlessManager::barrier(*this->_clusterBuffer, rawrbox::BufferType::UNORDERED_ACCESS);

			context->SetPipelineState(this->_clusterBuildingComputeProgram);
			context->CommitShaderResources(this->_clusterBuildingComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->DispatchCompute(DispatchAttribs);
		}
		// ------

		// Perform light / decal culling
		{
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
			DispatchAttribs.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
			DispatchAttribs.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Z, CLUSTERS_Z_THREADS);

			rawrbox::BindlessManager::barrier(*this->_clusterBuffer, rawrbox::BufferType::SHADER);
			rawrbox::BindlessManager::barrier(*this->_dataGridBuffer, rawrbox::BufferType::UNORDERED_ACCESS);

			context->SetPipelineState(this->_lightCullingComputeProgram);
			context->CommitShaderResources(this->_lightCullingComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			context->DispatchCompute(DispatchAttribs);
		}
		// ----------------------

		// Barrier ----
		rawrbox::BindlessManager::barrier(*this->_dataGridBuffer, rawrbox::BufferType::SHADER);
		rawrbox::BindlessManager::barrier(*this->_clusterBuffer, rawrbox::BufferType::SHADER);
		// ------------
	}

	void ClusteredPlugin::buildBuffers() {
		auto device = rawrbox::RENDERER->device();

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

			// Barrier ----
			rawrbox::BindlessManager::barrier(*this->_clusterBuffer, rawrbox::BufferType::UNORDERED_ACCESS);
			// ------------
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

			// Barrier ----
			rawrbox::BindlessManager::barrier(*this->_dataGridBuffer, rawrbox::BufferType::UNORDERED_ACCESS);
			// ------------
		}
		// ------------------
	}

	void ClusteredPlugin::buildPipelines() {
		// Load shader programs ---
		rawrbox::PipeComputeSettings settings;
		settings.macros = this->getClusterMacros();
		// -------------------------------

		// BUILDING -----
		settings.resources = {
		    {Diligent::SHADER_TYPE_COMPUTE, "Camera", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "Clusters", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::MAIN_CAMERA->uniforms(), "Camera"},
		    {Diligent::SHADER_TYPE_COMPUTE, this->_clusterBufferWrite, "Clusters"}};

		settings.pCS = "cluster_build.csh";
		settings.bind = "Cluster::Build";

		this->_clusterBuildingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Build", settings);
		this->_clusterBuildingComputeBind = rawrbox::PipelineUtils::getBind("Cluster::Build");
		// ---------

		// CULLING -----
		settings.resources = {
		    {Diligent::SHADER_TYPE_COMPUTE, "Camera", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "LightConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "DecalConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_COMPUTE, "ClusterDataGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "Clusters", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_COMPUTE, "Lights", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "Decals", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::MAIN_CAMERA->uniforms(), "Camera"},
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::LIGHTS::uniforms, "LightConstants"},
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::DECALS::uniforms, "DecalConstants"},

		    {Diligent::SHADER_TYPE_COMPUTE, this->_dataGridBufferWrite, "ClusterDataGrid"},
		    {Diligent::SHADER_TYPE_COMPUTE, this->_clusterBufferRead, "Clusters"},

		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::LIGHTS::getBuffer(), "Lights"},
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::DECALS::getBuffer(), "Decals"},
		};

		settings.pCS = "cluster_cull.csh";
		settings.bind = "Cluster::Cull";

		this->_lightCullingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Cull", settings);
		this->_lightCullingComputeBind = rawrbox::PipelineUtils::getBind("Cluster::Cull");
		//  ----
	}

	// UTILS ----
	Diligent::ShaderMacroHelper ClusteredPlugin::getClusterMacros() {
		Diligent::ShaderMacroHelper macro;

		macro.AddShaderMacro("THREAD_GROUP_SIZE", rawrbox::THREAD_GROUP_SIZE);
		macro.AddShaderMacro("GROUP_SIZE", GROUP_SIZE);
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
