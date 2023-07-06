
#include <rawrbox/render/model/light/manager.hpp>
#include <rawrbox/render/renderers/cluster.hpp>
#include <rawrbox/render/utils/render.hpp>

#define BGFX_DEFAULT_CLEAR (0 | BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH)

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
	}

	void RendererCluster::resize(const rawrbox::Vector2i& size) {
		RendererBase::resize(size);

		// Setup views -----
		bgfx::setViewRect(CLUSTER_BUILD_VIEW_ID, 0, 0, size.x, size.y);
		bgfx::setViewRect(LIGHT_CULL_VIEW_ID, 0, 0, size.x, size.y);
		// ----
	}

	void RendererCluster::render() {
		if (this->_uniforms == nullptr || this->worldRender == nullptr) return;

		// Set views
		auto view = rawrbox::MAIN_CAMERA->getViewMtx().data();
		auto proj = rawrbox::MAIN_CAMERA->getProjMtx();

		bgfx::setViewTransform(CLUSTER_BUILD_VIEW_ID, view, proj.data());
		bgfx::setViewTransform(LIGHT_CULL_VIEW_ID, view, proj.data());

		bgfx::setViewTransform(rawrbox::MAIN_DEFAULT_VIEW, view, proj.data());
		bgfx::setViewClear(rawrbox::MAIN_DEFAULT_VIEW, BGFX_DEFAULT_CLEAR, 1.0F, 0, 0);
		// bgfx::setViewFrameBuffer(rawrbox::MAIN_DEFAULT_VIEW, this->_frameBuffer);
		//  ---

		this->_uniforms->setUniforms(this->_size);

		// Only rebuild cluster if view changed
		if (this->_oldProj != proj) {
			this->_oldProj = proj;
			this->_uniforms->bindBuffers(false); // write access, all buffers

			bgfx::dispatch(CLUSTER_BUILD_VIEW_ID,
			    this->_clusterBuildingComputeProgram,
			    ClusterUniforms::CLUSTERS_X / ClusterUniforms::CLUSTERS_X_THREADS,
			    ClusterUniforms::CLUSTERS_Y / ClusterUniforms::CLUSTERS_Y_THREADS,
			    ClusterUniforms::CLUSTERS_Z / ClusterUniforms::CLUSTERS_Z_THREADS);
		}

		// reset atomic counter for light grid generation
		// buffers created with BGFX_BUFFER_COMPUTE_WRITE can't be updated from the CPU
		// this used to happen during cluster building when it was still run every frame
		this->_uniforms->bindBuffers(false); // write access, all buffers
		bgfx::dispatch(LIGHT_CULL_VIEW_ID, this->_resetCounterComputeProgram, 1, 1, 1);
		// --------

		// Light culling
		rawrbox::LIGHTS::bindUniforms();
		this->_uniforms->bindBuffers(false); // write access, all buffers

		bgfx::dispatch(LIGHT_CULL_VIEW_ID,
		    this->_lightCullingComputeProgram,
		    ClusterUniforms::CLUSTERS_X / ClusterUniforms::CLUSTERS_X_THREADS,
		    ClusterUniforms::CLUSTERS_Y / ClusterUniforms::CLUSTERS_Y_THREADS,
		    ClusterUniforms::CLUSTERS_Z / ClusterUniforms::CLUSTERS_Z_THREADS);
		// --------

		// Final Pass
		rawrbox::CURRENT_VIEW_ID = rawrbox::MAIN_DEFAULT_VIEW;
		bgfx::touch(rawrbox::MAIN_DEFAULT_VIEW); // Make sure we draw on the view

		// Render world ---
		this->worldRender();
		// ----------------

		rawrbox::RendererBase::postRender();
	}

	void RendererCluster::bindRenderUniforms() {
		rawrbox::LIGHTS::bindUniforms();
		this->_uniforms->bindBuffers(true); // read access, only light grid and indices
	}

	// Is it supported by the GPU?
	bool RendererCluster::supported() {
		const bgfx::Caps* caps = bgfx::getCaps();
		return RendererBase::supported() &&
		       // compute shader
		       (caps->supported & BGFX_CAPS_COMPUTE) != 0 &&
		       // 32-bit index buffers, used for light grid structure
		       (caps->supported & BGFX_CAPS_INDEX32) != 0;
	}
} // namespace rawrbox
