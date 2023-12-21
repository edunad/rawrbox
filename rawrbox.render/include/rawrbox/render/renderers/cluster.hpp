#pragma once
/*#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

// Convert to passes
// https://github.com/HexaEngine/HexaEngine/blob/164bce63d837c41e7a2103c10fa1bf885ba48662/HexaEngine/Graphics/Passes/LightCullPass.cs#L24
namespace rawrbox {
	static constexpr uint32_t CLUSTER_TEXTEL_SIZE = 64;
	static constexpr uint32_t CLUSTERS_Z = 32;

	static constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 256;
	static constexpr uint32_t CLUSTERED_LIGHTING_NUM_BUCKETS = MAX_LIGHTS_PER_CLUSTER / CLUSTERS_Z;

	static constexpr uint32_t CLUSTERS_X_THREADS = 4;
	static constexpr uint32_t CLUSTERS_Y_THREADS = 4;
	static constexpr uint32_t CLUSTERS_Z_THREADS = 4;

	static constexpr uint32_t THREAD_GROUP_SIZE = CLUSTERS_X_THREADS * CLUSTERS_Y_THREADS * CLUSTERS_Z_THREADS;

	struct ClusterAABB {
		rawrbox::Vector4f minBounds = {};
		rawrbox::Vector4f maxBounds = {};
	};

	struct ClusterUniforms {
		rawrbox::Vector2f g_ZNearFarVec = {};
	};

	struct ClusterCullConstants {
		rawrbox::Matrix4x4 g_View = {};
	};

	struct ClusterBuildConstants : public rawrbox::ClusterUniforms {
		rawrbox::Vector2f g_ScreenSizeInv = {};
		rawrbox::Matrix4x4 g_InvProj = {};
	};

	class RendererCluster : public rawrbox::RendererBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _buildUniforms;
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _cullUniforms;

		Diligent::IPipelineState* _clusterBuildingComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _clusterBuildingComputeBind = nullptr;

		Diligent::IPipelineState* _clusterResetComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _clusterResetComputeBind = nullptr;

		Diligent::IPipelineState* _lightCullingComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _lightCullingComputeBind = nullptr;

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _clusterBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _dataGridBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferRead;
		// -----------

		rawrbox::Matrix4x4 _oldProj = {};

	public:
		static uint32_t CLUSTERS_X;
		static uint32_t CLUSTERS_Y;

		static uint32_t GROUP_SIZE;

		RendererCluster(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Vector2i& screenSize, const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
		RendererCluster(const RendererCluster&) = delete;
		RendererCluster(RendererCluster&&) = delete;
		RendererCluster& operator=(const RendererCluster&) = delete;
		RendererCluster& operator=(RendererCluster&&) = delete;
		~RendererCluster() override;

		void init(Diligent::DeviceFeatures features = {}) override;
		void resize(const rawrbox::Vector2i& size, const rawrbox::Vector2i& screenSize) override;
		void render() override;

		// UTILS ----
		Diligent::ShaderMacroHelper getClusterMacros();

		Diligent::IBufferView* getClustersBuffer(bool readOnly = true);
		Diligent::IBufferView* getDataGridBuffer(bool readOnly = true);
		// ----------

		template <typename T>
		void bindUniforms(Diligent::MapHelper<T>& helper) {
			(*helper).g_ZNearFarVec = {this->_camera->getZNear(), this->_camera->getZFar()};
		}
	};
} // namespace rawrbox
*/
