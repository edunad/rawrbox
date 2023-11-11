
// #include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/materials/lit.hpp>
#include <rawrbox/render/renderers/cluster.hpp>
#include <rawrbox/render/utils/render.hpp>

namespace rawrbox {

	RendererCluster::RendererCluster(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Colorf& clearColor) : rawrbox::RendererBase(type, window, size, clearColor) {}
	RendererCluster::~RendererCluster() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system

		RAWRBOX_DESTROY(this->_uniforms);

		RAWRBOX_DESTROY(this->_atomicIndexBuffer);
		RAWRBOX_DESTROY(this->_atomicIndexBufferWrite);
		RAWRBOX_DESTROY(this->_atomicIndexBufferRead);

		RAWRBOX_DESTROY(this->_clusterBuffer);
		RAWRBOX_DESTROY(this->_clusterBufferWrite);
		RAWRBOX_DESTROY(this->_clusterBufferRead);

		RAWRBOX_DESTROY(this->_lightIndicesBuffer);
		RAWRBOX_DESTROY(this->_lightIndicesBufferWrite);
		RAWRBOX_DESTROY(this->_lightIndicesBufferRead);

		RAWRBOX_DESTROY(this->_lightGridsBuffer);
		RAWRBOX_DESTROY(this->_lightGridsBufferWrite);
		RAWRBOX_DESTROY(this->_lightGridsBufferRead);
	}

	void RendererCluster::completeIntro() {
		// Init & load materials ---
		rawrbox::MaterialLit::init();
		// -----

		rawrbox::RendererBase::completeIntro();
	}

	// -------------------------------------------
	void RendererCluster::init(Diligent::DeviceFeatures features) {
		auto check = CLUSTERS_Z % CLUSTERS_Z_THREADS != 0;
		if (check) throw std::runtime_error("[RawrBox-Renderer] Number of cluster depth slices must be divisible by thread count z-dimension");

		features.ComputeShaders = Diligent::DEVICE_FEATURE_STATE_ENABLED;
		RendererBase::init(features);

		// Initialize light engine
		rawrbox::LIGHTS::init();

		// Init uniforms
		Diligent::BufferDesc BuffDesc;
		BuffDesc.Name = "rawrbox::Cluster::Uniforms";
		BuffDesc.Usage = Diligent::USAGE_DYNAMIC;
		BuffDesc.BindFlags = Diligent::BIND_UNIFORM_BUFFER;
		BuffDesc.CPUAccessFlags = Diligent::CPU_ACCESS_WRITE;
		BuffDesc.Size = sizeof(rawrbox::ClusterConstants);

		this->_device->CreateBuffer(BuffDesc, nullptr, &this->_uniforms);
		// -----------------------------------------

		// Create buffers --

		// Atomic index --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
			BuffDesc.Size = BuffDesc.ElementByteStride * rawrbox::CLUSTER_COUNT; // Maybe?
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_atomicIndexBuffer);

			Diligent::BufferViewDesc ViewDesc;
			ViewDesc.ViewType = Diligent::BUFFER_VIEW_UNORDERED_ACCESS;
			ViewDesc.Format.ValueType = Diligent::VT_UINT32;
			ViewDesc.Format.NumComponents = 1;

			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_atomicIndexBufferWrite); // Write / Read

			ViewDesc.ViewType = Diligent::BUFFER_VIEW_SHADER_RESOURCE;
			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_atomicIndexBufferRead); // Read only
		}
		// --------------

		// Clusters  --
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(rawrbox::Cluster);
			BuffDesc.Usage = Diligent::USAGE_DEFAULT;
			BuffDesc.Mode = Diligent::BUFFER_MODE_STRUCTURED;
			BuffDesc.Size = BuffDesc.ElementByteStride * rawrbox::CLUSTER_COUNT;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_clusterBuffer);

			this->_clusterBufferWrite = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_UNORDERED_ACCESS); // Write / Read
			this->_clusterBufferRead = this->_clusterBuffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE);   //  Read only
		}
		// --------------

		// Light indices ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(uint32_t);
			BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
			BuffDesc.Size = BuffDesc.ElementByteStride * rawrbox::CLUSTER_COUNT * rawrbox::MAX_LIGHTS_PER_CLUSTER;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_lightIndicesBuffer);

			Diligent::BufferViewDesc ViewDesc;
			ViewDesc.ViewType = Diligent::BUFFER_VIEW_UNORDERED_ACCESS;
			ViewDesc.Format.ValueType = Diligent::VT_UINT32;
			ViewDesc.Format.NumComponents = 1;

			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_lightIndicesBufferWrite); // Write / Read

			ViewDesc.ViewType = Diligent::BUFFER_VIEW_SHADER_RESOURCE;
			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_lightIndicesBufferRead); // Read only
		}
		// ------------------

		// Light grid ---
		{
			Diligent::BufferDesc BuffDesc;
			BuffDesc.ElementByteStride = sizeof(std::array<uint32_t, 2>);
			BuffDesc.Mode = Diligent::BUFFER_MODE_FORMATTED;
			BuffDesc.Size = BuffDesc.ElementByteStride * rawrbox::CLUSTER_COUNT;
			BuffDesc.BindFlags = Diligent::BIND_UNORDERED_ACCESS | Diligent::BIND_SHADER_RESOURCE;

			this->_device->CreateBuffer(BuffDesc, nullptr, &this->_lightGridsBuffer);

			Diligent::BufferViewDesc ViewDesc;
			ViewDesc.ViewType = Diligent::BUFFER_VIEW_UNORDERED_ACCESS;
			ViewDesc.Format.ValueType = Diligent::VT_UINT32;
			ViewDesc.Format.NumComponents = 1;

			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_lightGridsBufferWrite); // Write / Read

			ViewDesc.ViewType = Diligent::BUFFER_VIEW_SHADER_RESOURCE;
			this->_atomicIndexBuffer->CreateView(ViewDesc, &this->_lightGridsBufferRead); // Read only
		}
		// ------------------

		// -----------------

		// Load shader programs ---
		rawrbox::PipeComputeSettings settings;

		settings.macros.AddShaderMacro("THREAD_GROUP_SIZE", rawrbox::THREAD_CLUSTER_COUNT);
		settings.macros.AddShaderMacro("GROUP_SIZE", rawrbox::CLUSTER_COUNT);
		settings.macros.AddShaderMacro("CLUSTERS_X_THREADS", rawrbox::CLUSTERS_X_THREADS);
		settings.macros.AddShaderMacro("CLUSTERS_Y_THREADS", rawrbox::CLUSTERS_Y_THREADS);
		settings.macros.AddShaderMacro("CLUSTERS_Z_THREADS", rawrbox::CLUSTERS_Z_THREADS);
		settings.macros.AddShaderMacro("CLUSTERS_X", rawrbox::CLUSTERS_X);
		settings.macros.AddShaderMacro("CLUSTERS_Y", rawrbox::CLUSTERS_Y);
		settings.macros.AddShaderMacro("CLUSTERS_Z", rawrbox::CLUSTERS_Z);
		settings.macros.AddShaderMacro("MAX_LIGHTS_PER_CLUSTER", rawrbox::MAX_LIGHTS_PER_CLUSTER);

		settings.pCS = "reset.csh";
		this->_resetCounterComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Clustered::Reset", "Clustered::Reset", settings);

		// Bind ---
		this->_resetCounterComputeBind = rawrbox::PipelineUtils::getBind("Clustered::Reset");
		this->_resetCounterComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_globalIndex")->Set(this->_atomicIndexBufferWrite);
		// --------

		settings.resources = {{Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_COMPUTE, this->_uniforms}};
		settings.pCS = "cluster.csh";
		this->_clusterBuildingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Clustered::Building", "Clustered::Building", settings);

		// Bind ----
		this->_clusterBuildingComputeBind = rawrbox::PipelineUtils::getBind("Clustered::Building");
		this->_clusterBuildingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_Clusters")->Set(this->_clusterBufferWrite);
		// ---------

		settings.resources = {{Diligent::SHADER_TYPE_COMPUTE, "Constants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}, {Diligent::SHADER_TYPE_COMPUTE, "LightConstants", Diligent::SHADER_RESOURCE_VARIABLE_TYPE_STATIC}};
		settings.uniforms = {{Diligent::SHADER_TYPE_COMPUTE, this->_uniforms, "Constants"}, {Diligent::SHADER_TYPE_COMPUTE, rawrbox::LIGHTS::uniforms, "LightConstants"}};
		settings.pCS = "culling.csh";
		this->_lightCullingComputeProgram = rawrbox::PipelineUtils::createComputePipeline("Clustered::LightCull", "Clustered::LightCull", settings);
		//  ----

		// Bind ----
		this->_lightCullingComputeBind = rawrbox::PipelineUtils::getBind("Clustered::LightCull");
		this->_lightCullingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_clusterLightGrid")->Set(this->_lightGridsBufferWrite);
		this->_lightCullingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_clusterLightIndices")->Set(this->_lightIndicesBufferWrite);
		this->_lightCullingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_globalIndex")->Set(this->_atomicIndexBufferWrite);
		this->_lightCullingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_Clusters")->Set(this->_clusterBufferRead);
		this->_lightCullingComputeBind->GetVariableByName(Diligent::SHADER_TYPE_COMPUTE, "g_Lights")->Set(rawrbox::LIGHTS::buffer->GetDefaultView(Diligent::BUFFER_VIEW_SHADER_RESOURCE));
		// ---------
	}

	void RendererCluster::resize(const rawrbox::Vector2i& size) {
		RendererBase::resize(size);
	}

	void RendererCluster::render() {
		if (this->_uniforms == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Render uniforms not set! Did you call 'init' ?");
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

		// SETUP CLUSTER UNIFORMS ----------------------------
		{
			Diligent::MapHelper<rawrbox::ClusterConstants> CBConstants(this->_context, this->_uniforms, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);

			auto size = this->_size.cast<float>();
			auto tInvProj = this->_camera->getProjMtx();
			auto tView = this->_camera->getViewMtx().transpose();
			tInvProj.inverse();

			CBConstants->g_ScreenSize = {0, 0, size.x, size.y};
			CBConstants->g_InvProj = tInvProj;
			CBConstants->g_View = tView;

			this->bindUniforms<rawrbox::ClusterConstants>(CBConstants);
		}

		// Setup light uniforms
		rawrbox::LIGHTS::bindUniforms();
		// ------------

		// Only rebuild cluster if view changed
		auto proj = this->_camera->getProjMtx();

		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			// Rebuild clusters ---
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = CLUSTERS_X / CLUSTERS_X_THREADS;
			DispatchAttribs.ThreadGroupCountY = CLUSTERS_Y / CLUSTERS_Y_THREADS;
			DispatchAttribs.ThreadGroupCountZ = CLUSTERS_Z / CLUSTERS_Z_THREADS;

			this->_context->SetPipelineState(this->_clusterBuildingComputeProgram);
			this->_context->CommitShaderResources(this->_clusterBuildingComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->_context->DispatchCompute(DispatchAttribs);
		}
		// ------

		// reset atomic counter for light grid generation
		{
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = 1;
			DispatchAttribs.ThreadGroupCountY = 1;
			DispatchAttribs.ThreadGroupCountZ = 1;

			this->_context->SetPipelineState(this->_resetCounterComputeProgram);
			this->_context->CommitShaderResources(this->_resetCounterComputeBind, Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
			this->_context->DispatchCompute(DispatchAttribs);
		}
		//  --------

		// Perform light / decal culling
		{
			Diligent::DispatchComputeAttribs DispatchAttribs;
			DispatchAttribs.ThreadGroupCountX = CLUSTERS_X / CLUSTERS_X_THREADS;
			DispatchAttribs.ThreadGroupCountY = CLUSTERS_Y / CLUSTERS_Y_THREADS;
			DispatchAttribs.ThreadGroupCountZ = CLUSTERS_Z / CLUSTERS_Z_THREADS;

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

		/*if (this->_uniforms == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Render uniforms not set! Did you call 'init' ?");
		if (this->worldRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] World render method not set! Did you call 'setWorldRender' ?");
		if (this->overlayRender == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Overlay render method not set! Did you call 'setOverlayRender' ?");

		// No world / overlay only
		if (rawrbox::MAIN_CAMERA == nullptr) {
			rawrbox::RendererBase::render();
			return;
		}

		// Set views
		auto view = rawrbox::MAIN_CAMERA->getViewMtx();
		auto proj = rawrbox::MAIN_CAMERA->getProjMtx();

		bgfx::setViewTransform(CLUSTER_BUILD_VIEW_ID, view.data(), proj.data());
		bgfx::setViewTransform(LIGHT_CULL_VIEW_ID, view.data(), proj.data());
		//     ---

		this->_uniforms->setUniforms(this->_size);



		// reset atomic counter for light grid generation
		// buffers created with BGFX_BUFFER_COMPUTE_WRITE can't be updated from the CPU
		// this used to happen during cluster building when it was still run every frame
		this->_uniforms->bindAtomic();
		bgfx::dispatch(LIGHT_CULL_VIEW_ID, this->_resetCounterComputeProgram, 1, 1, 1);
		// --------

		// Light culling
		rawrbox::LIGHTS::bindUniforms();

		this->_uniforms->bindCluster(true);
		this->_uniforms->bindLightGrid();
		this->_uniforms->bindAtomic();
		this->_uniforms->bindLightIndices();

		bgfx::dispatch(LIGHT_CULL_VIEW_ID,
		    this->_lightCullingComputeProgram,
		    ClusterUniforms::CLUSTERS_X / ClusterUniforms::CLUSTERS_X_THREADS,
		    ClusterUniforms::CLUSTERS_Y / ClusterUniforms::CLUSTERS_Y_THREADS,
		    ClusterUniforms::CLUSTERS_Z / ClusterUniforms::CLUSTERS_Z_THREADS);
		// --------

		// Final Pass -------------
		this->finalRender();
		// ------------------------

		// Check GPU Picking -----
		this->gpuCheck();
		// -------------------

		this->frame(); // Submit ---*/
	}

	/*void RendererCluster::bindRenderUniforms() {
		rawrbox::LIGHTS::bindUniforms();

		this->_uniforms->bindLightIndices(true);
		this->_uniforms->bindLightGrid(true);
	}*/

	// UTILS ----
	Diligent::IBufferView* RendererCluster::getAtomicIndexBuffer(bool readOnly) { return readOnly ? this->_atomicIndexBufferRead : this->_atomicIndexBufferWrite; }
	Diligent::IBufferView* RendererCluster::getClustersBuffer(bool readOnly) { return readOnly ? this->_clusterBufferRead : this->_clusterBufferWrite; }
	// ----------
} // namespace rawrbox
