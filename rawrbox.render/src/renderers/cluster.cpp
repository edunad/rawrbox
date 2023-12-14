
// #include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/renderers/cluster.hpp>
#include <rawrbox/render/utils/render.hpp>

namespace rawrbox {

	uint32_t RendererCluster::CLUSTERS_X = 0;
	uint32_t RendererCluster::CLUSTERS_Y = 0;
	uint32_t RendererCluster::GROUP_SIZE = 0;

	RendererCluster::RendererCluster(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Vector2i& screenSize, const rawrbox::Colorf& clearColor) : rawrbox::RendererBase(type, window, size, screenSize, clearColor) {
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(size.x, CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(size.y, CLUSTER_TEXTEL_SIZE);

		GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;
	}

	RendererCluster::~RendererCluster() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system

		RAWRBOX_DESTROY(this->_buildUniforms);
		RAWRBOX_DESTROY(this->_cullUniforms);

		RAWRBOX_DESTROY(this->_clusterBuffer);
		RAWRBOX_DESTROY(this->_clusterBufferWrite);
		RAWRBOX_DESTROY(this->_clusterBufferRead);

		RAWRBOX_DESTROY(this->_dataGridBuffer);
		RAWRBOX_DESTROY(this->_dataGridBufferWrite);
		RAWRBOX_DESTROY(this->_dataGridBufferRead);
	}

	// -------------------------------------------
	void RendererCluster::init(Diligent::DeviceFeatures features) {
		auto check = CLUSTERS_Z % CLUSTERS_Z_THREADS != 0;
		if (check) throw std::runtime_error("[RawrBox-Renderer] Number of cluster depth slices must be divisible by thread count z-dimension");

		features.ComputeShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.BindlessResources = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		features.SparseResources = Diligent::DEVICE_FEATURE_STATE_ENABLED;

		RendererBase::init(features);

		// Initialize light engine
		rawrbox::LIGHTS::init();

		// Init uniforms
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Cluster::Build::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffDesc.Size = sizeof(rawrbox::ClusterBuildConstants);

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_buildUniforms);
		}

		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.Name = "rawrbox::Cluster::Cull::Uniforms";
			BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
			BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
			BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
			BuffDesc.Size = sizeof(rawrbox::ClusterCullConstants);

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_cullUniforms);
		}
		// -----------------------------------------

		// Clusters  --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::ClusterAABB);
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Name = "rawrbox::Cluster::Cluster";
			BuffDesc.Size = BuffDesc.ElementByteStride * GROUP_SIZE;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_clusterBuffer);

			this->_clusterBufferWrite = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_clusterBufferRead = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// --------------

		// Data grid ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
			BuffDesc.Name = "rawrbox::Cluster::ClusterDataGrid";
			BuffDesc.Size = BuffDesc.ElementByteStride * (rawrbox::MAX_LIGHTS_PER_CLUSTER / rawrbox::CLUSTERS_Z * GROUP_SIZE);
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_dataGridBuffer);

			Diligent::BufferViewDesc ViewDesc;
			ViewDesc.ViewType = Diligent::BUFFER_VIEW_UNORDERED_ACCESS;
			ViewDesc.Format.ValueType = Diligent::VT_UINT32;
			ViewDesc.Format.NumComponents = 1;

			this->_dataGridBuffer->CreateView(ViewDesc, &this->_dataGridBufferWrite); // Write / Read

			ViewDesc.ViewType = Diligent::BUFFER_VIEW_SHADER_RESOURCE;
			this->_dataGridBuffer->CreateView(ViewDesc, &this->_dataGridBufferRead); // Read only
		}

		// ------------------

		// Load shader programs ---
		rawrbox::PipeComputeSettings settings;
		settings.macros = this->getClusterMacros();
		// -------------------------------

		// CLUSTER BUILDING -----
		settings.resources = {{Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}, {Diligent::SHADER_TYPE_COMPUTE, "g_Clusters", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
		settings.uniforms = {
		    {Diligent::SHADER_TYPE_COMPUTE, this->_buildUniforms, "Constants"},
		    {Diligent::SHADER_TYPE_COMPUTE, this->_clusterBufferWrite, "g_Clusters"}};
		settings.pCS = "cluster_build.csh";

		this->_clusterBuildingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Build", "Cluster::Build", settings);
		this->_clusterBuildingComputeBind = rawrbox::PipelineUtils::getBind("Cluster::Build");
		// ---------

		// CLUSTER BUILDING -----
		settings.resources = {{Diligent::SHADER_TYPE_COMPUTE, "g_ClusterDataGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
		settings.uniforms = {
		    {Diligent::SHADER_TYPE_COMPUTE, this->_dataGridBufferWrite, "g_ClusterDataGrid"}};
		settings.pCS = "cluster_reset.csh";

		this->_clusterResetComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Reset", "Cluster::Reset", settings);
		this->_clusterResetComputeBind = rawrbox::PipelineUtils::getBind("Cluster::Reset");
		// ---------

		// CULLING -----
		settings.resources = {
		    {Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "LightConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

		    {Diligent::SHADER_TYPE_COMPUTE, "g_ClusterDataGrid", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "g_Clusters", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
		    {Diligent::SHADER_TYPE_COMPUTE, "g_Lights", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};

		settings.uniforms = {
		    {Diligent::SHADER_TYPE_COMPUTE, this->_cullUniforms, "Constants"},
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::LIGHTS::uniforms, "LightConstants"},

		    {Diligent::SHADER_TYPE_COMPUTE, this->_dataGridBufferWrite, "g_ClusterDataGrid"},
		    {Diligent::SHADER_TYPE_COMPUTE, this->_clusterBufferRead, "g_Clusters"},
		    {Diligent::SHADER_TYPE_COMPUTE, rawrbox::LIGHTS::getBuffer(), "g_Lights"},
		};

		settings.pCS = "cluster_cull.csh";
		this->_lightCullingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Cluster::Cull", "Cluster::Cull", settings);
		this->_lightCullingComputeBind = rawrbox::PipelineUtils::getBind("Cluster::Cull");
		//  ----

		// Init & load materials ---
		rawrbox::MaterialLit::init();
		// -----
	}

	void RendererCluster::resize(const rawrbox::Vector2i& size, const rawrbox::Vector2i& screenSize) {
		// todo: decal resize
		RendererBase::resize(size, screenSize);

		// Re-calculate clusters ----
		CLUSTERS_X = rawrbox::MathUtils::divideRound<uint32_t>(size.x, CLUSTER_TEXTEL_SIZE);
		CLUSTERS_Y = rawrbox::MathUtils::divideRound<uint32_t>(size.y, CLUSTER_TEXTEL_SIZE);

		GROUP_SIZE = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;
		// --------------------------

		this->_oldProj = {}; // Re-build clusters
	}

	void RendererCluster::render() {
		if (this->_cullUniforms == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Render uniforms not set! Did you call 'init' ?");
		if (this->_buildUniforms == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Render uniforms not set! Did you call 'init' ?");

		if (this->_swapChain == nullptr || this->_context == nullptr || this->_device == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Failed to bind swapChain/context/device! Did you call 'init' ?");

		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// Clear backbuffer ----
		this->clear();
		// ---------------------

		// No world / overlay only
		if (this->_camera == nullptr) {
			this->overlayRender();
			this->frame();
			return;
		}

		// Setup light uniforms
		rawrbox::LIGHTS::bindUniforms();
		// ------------

		// Only rebuild cluster if view changed
		auto proj = this->_camera->getProjMtx();
		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			// SETUP BUILD CLUSTER UNIFORMS ----------------------------
			{
				rawrbox::Vector2f size = this->_size.cast<float>();

				Diligent::MapHelper<rawrbox::ClusterBuildConstants> CBConstants(this->_context, this->_buildUniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				CBConstants->g_ScreenSizeInv = {1.0F / size.x, 1.0F / size.y};
				CBConstants->g_InvProj = proj.inverse();

				this->bindUniforms<rawrbox::ClusterBuildConstants>(CBConstants);
			}
			// -----

			// Rebuild clusters ---
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
			DispatchAttribs.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
			DispatchAttribs.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Z, CLUSTERS_Z_THREADS);

			this->_context->SetPipelineState(this->_clusterBuildingComputeProgram);
			this->_context->CommitShaderResources(this->_clusterBuildingComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->_context->DispatchCompute(DispatchAttribs);
		}
		// ------

		/*{
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = 1;
			DispatchAttribs.ThreadGroupCountY = 1;
			DispatchAttribs.ThreadGroupCountZ = 1;

			this->_context->SetPipelineState(this->_clusterResetComputeProgram);
			this->_context->CommitShaderResources(this->_clusterResetComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->_context->DispatchCompute(DispatchAttribs);
		}*/

		// Perform light / decal culling
		{
			// SETUP CULLING CLUSTER UNIFORMS ----------------------------
			{
				Diligent::MapHelper<rawrbox::ClusterCullConstants> CBConstants(this->_context, this->_cullUniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				auto view = this->_camera->getViewMtx();
				CBConstants->g_View = view;
			}
			// -----

			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_X, CLUSTERS_X_THREADS);
			DispatchAttribs.ThreadGroupCountY = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Y, CLUSTERS_Y_THREADS);
			DispatchAttribs.ThreadGroupCountZ = rawrbox::MathUtils::divideRound<uint32_t>(CLUSTERS_Z, CLUSTERS_Z_THREADS);

			this->_context->SetPipelineState(this->_lightCullingComputeProgram);
			this->_context->CommitShaderResources(this->_lightCullingComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->_context->DispatchCompute(DispatchAttribs);
		}

		// Final Pass -------------
		this->finalRender();
		// ------------------------

		// Check GPU Picking -----
		// this->gpuCheck();
		// -------------------

		// Submit ---
		this->frame();
		// ----------
	}

	// UTILS ----
	Diligent::ShaderMacroHelper RendererCluster::getClusterMacros() {
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
		macro.AddShaderMacro("CLUSTERED_LIGHTING_NUM_BUCKETS", rawrbox::CLUSTERED_LIGHTING_NUM_BUCKETS);

		return macro;
	}

	Diligent::IBufferView* RendererCluster::getClustersBuffer(bool readOnly) { return readOnly ? this->_clusterBufferRead : this->_clusterBufferWrite; }
	Diligent::IBufferView* RendererCluster::getDataGridBuffer(bool readOnly) { return readOnly ? this->_dataGridBufferRead : this->_dataGridBufferWrite; }
	// ----------
} // namespace rawrbox
