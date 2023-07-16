
#include <rawrbox/render/decals/manager.hpp>
#include <rawrbox/render/light/manager.hpp>
#include <rawrbox/render/renderers/cluster.hpp>
#include <rawrbox/render/utils/render.hpp>

// NOLINTBEGIN(*)
const bgfx::EmbeddedShader clustered_clusterbuilding[] = {
    BGFX_EMBEDDED_SHADER(cs_clustered_clusterbuilding),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_reset_counter[] = {
    BGFX_EMBEDDED_SHADER(cs_clustered_reset_counter),
    BGFX_EMBEDDED_SHADER_END()};

const bgfx::EmbeddedShader clustered_lightculling[] = {
    BGFX_EMBEDDED_SHADER(cs_clustered_lightculling),
    BGFX_EMBEDDED_SHADER_END()};
// NOLINTEND(*)

constexpr bgfx::ViewId CLUSTER_BUILD_VIEW_ID = 0;
constexpr bgfx::ViewId LIGHT_CULL_VIEW_ID = 1;

namespace rawrbox {
	RendererCluster::~RendererCluster() {
		rawrbox::LIGHTS::shutdown(); // Shutdown light system

		RAWRBOX_DESTROY(this->_clusterBuildingComputeProgram);
		RAWRBOX_DESTROY(this->_lightCullingComputeProgram);
		RAWRBOX_DESTROY(this->_resetCounterComputeProgram);

		this->_uniforms.reset();
	}

	// -------------------------------------------
	void RendererCluster::init(const rawrbox::Vector2i& size) {
		RendererBase::init(size);

		this->_uniforms = std::make_unique<rawrbox::ClusterUniforms>();
		this->_uniforms->initialize();

		// Load shader programs ---
		rawrbox::RenderUtils::buildComputeShader(clustered_clusterbuilding, this->_clusterBuildingComputeProgram);
		rawrbox::RenderUtils::buildComputeShader(clustered_reset_counter, this->_resetCounterComputeProgram);
		rawrbox::RenderUtils::buildComputeShader(clustered_lightculling, this->_lightCullingComputeProgram);
		// ----

		bgfx::setViewName(CLUSTER_BUILD_VIEW_ID, "RAWRBOX-CLUSTER-COMPUTE");
		bgfx::setViewName(LIGHT_CULL_VIEW_ID, "RAWRBOX-LIGHT-CULL-COMPUTE");

		// Initialize light engine
		rawrbox::LIGHTS::init();
		rawrbox::DECALS::init();
	}

	void RendererCluster::resize(const rawrbox::Vector2i& size) {
		RendererBase::resize(size);

		// Setup views -----
		bgfx::setViewRect(CLUSTER_BUILD_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewRect(LIGHT_CULL_VIEW_ID, 0, 0, size.x, size.y);
		// ----
	}

	void RendererCluster::render() {
		if (this->_uniforms == nullptr) throw std::runtime_error("[Rawrbox-Renderer] Render uniforms not set! Did you call 'init' ?");
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

		// Only rebuild cluster if view changed
		if (this->_oldProj != proj) {
			this->_oldProj = proj;

			this->_uniforms->bindCluster();
			bgfx::dispatch(CLUSTER_BUILD_VIEW_ID,
			    this->_clusterBuildingComputeProgram,
			    ClusterUniforms::CLUSTERS_X / ClusterUniforms::CLUSTERS_X_THREADS,
			    ClusterUniforms::CLUSTERS_Y / ClusterUniforms::CLUSTERS_Y_THREADS,
			    ClusterUniforms::CLUSTERS_Z / ClusterUniforms::CLUSTERS_Z_THREADS);
		}
		// ------

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

		// Final Pass ---------------------

		// Record world ---
		this->_render->startRecord();
		this->worldRender();
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, rawrbox::MAIN_CAMERA->getViewMtx().data(), rawrbox::MAIN_CAMERA->getProjMtx().data());
		this->_render->stopRecord();
		// ----------------

		// Render world ---
		bgfx::ViewId prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_WORLD_VIEW;
		// ---
		rawrbox::RenderUtils::drawQUAD(this->_render->getHandle(), this->_size);
		// -----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		// Render overlay ---
		prevId = rawrbox::CURRENT_VIEW_ID;
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_OVERLAY_VIEW;
		// ---
		bgfx::setViewTransform(rawrbox::CURRENT_VIEW_ID, nullptr, nullptr);
		this->overlayRender();
		// ----------------

		// Restore id -----
		rawrbox::CURRENT_VIEW_ID = prevId;
		bgfx::discard(BGFX_DISCARD_ALL);
		// ------------------------

		this->frame(); // Submit ---
	}

	void RendererCluster::bindRenderUniforms() {
		rawrbox::LIGHTS::bindUniforms();

		this->_uniforms->bindLightIndices(true);
		this->_uniforms->bindLightGrid(true);
	}

	bool RendererCluster::supported() {
		const bgfx::Caps* caps = bgfx::getCaps();

		return RendererBase::supported() &&
		       (caps->supported & BGFX_CAPS_COMPUTE) != 0 &&
		       (caps->supported & BGFX_CAPS_INDEX32) != 0;
	}
} // namespace rawrbox
