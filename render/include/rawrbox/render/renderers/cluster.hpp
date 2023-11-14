#pragma once
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

namespace rawrbox {
	// taken from Doom
	// http://advances.realtimerendering.com/s2016/Siggraph2016_idTech6.pdf
	static constexpr uint32_t CLUSTERS_X = 16;
	static constexpr uint32_t CLUSTERS_Y = 8;
	static constexpr uint32_t CLUSTERS_Z = 24;

	// limit number of threads (D3D only allows up to 1024, there might also be shared memory limitations)
	// shader will be run by 6 work groups
	static constexpr uint32_t CLUSTERS_X_THREADS = 16;
	static constexpr uint32_t CLUSTERS_Y_THREADS = 8;
	static constexpr uint32_t CLUSTERS_Z_THREADS = 4;

	static constexpr uint32_t CLUSTER_COUNT = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;
	static constexpr uint32_t THREAD_CLUSTER_COUNT = CLUSTERS_X_THREADS * CLUSTERS_Y_THREADS * CLUSTERS_Z_THREADS;

	static constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 100;

	struct Cluster {
		// w is padding
		rawrbox::Vector3f minBounds = {};
		rawrbox::Vector3f maxBounds = {};

		static std::array<Diligent::LayoutElement, 2> vLayout() {
			return {
			    // Attribute 0 - MinBounds
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - MaxBounds
			    Diligent::LayoutElement{1, 0, 3, Diligent::VT_FLOAT32, false},
			};
		}
	};

	struct ClusterUniforms {
		rawrbox::Vector2f g_ClusterSize = {};
		rawrbox::Vector2f g_ZNearFarVec = {};
	};

	struct ClusterConstants : public rawrbox::ClusterUniforms {
		// CAMERA ------
		rawrbox::Vector4f g_ScreenSize = {};
		rawrbox::Matrix4x4 g_InvProj = {};
		rawrbox::Matrix4x4 g_View = {};
		// --------------
	};

	class RendererCluster : public rawrbox::RendererBase {
	protected:
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _uniforms;

		Diligent::IPipelineState* _clusterBuildingComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _clusterBuildingComputeBind = nullptr;

		Diligent::IPipelineState* _resetCounterComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _resetCounterComputeBind = nullptr;

		Diligent::IPipelineState* _lightCullingComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _lightCullingComputeBind = nullptr;

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _atomicIndexBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _atomicIndexBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _atomicIndexBufferRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _clusterBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _lightIndicesBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _lightIndicesBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _lightIndicesBufferRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _dataGridBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferRead;
		// -----------

		rawrbox::Matrix4x4 _oldProj = {};

	public:
		RendererCluster(Diligent::RENDER_DEVICE_TYPE type, Diligent::NativeWindow window, const rawrbox::Vector2i& size, const rawrbox::Colorf& clearColor = rawrbox::Colors::Black());
		RendererCluster(const RendererCluster&) = delete;
		RendererCluster(RendererCluster&&) = delete;
		RendererCluster& operator=(const RendererCluster&) = delete;
		RendererCluster& operator=(RendererCluster&&) = delete;
		~RendererCluster() override;

		void init(Diligent::DeviceFeatures features = {}) override;
		void resize(const rawrbox::Vector2i& size) override;
		void render() override;

		// UTILS ----
		Diligent::IBufferView* getAtomicIndexBuffer(bool readOnly = true);
		Diligent::IBufferView* getClustersBuffer(bool readOnly = true);
		Diligent::IBufferView* getLightIndicesBuffer(bool readOnly = true);
		Diligent::IBufferView* getDataGridBuffer(bool readOnly = true);
		// ----------

		template <typename T>
		void bindUniforms(Diligent::MapHelper<T>& helper) {
			auto size = this->_size.cast<float>();

			(*helper).g_ClusterSize = {std::ceil((float)size.x / CLUSTERS_X),
			    std::ceil((float)size.y / CLUSTERS_Y)};
			(*helper).g_ZNearFarVec = {this->_camera->getZNear(), this->_camera->getZFar()};
		}
	};
} // namespace rawrbox
