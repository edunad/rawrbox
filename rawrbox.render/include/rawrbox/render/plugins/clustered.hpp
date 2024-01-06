#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/plugins/base.hpp>

#include <RefCntAutoPtr.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>

namespace rawrbox {
	static constexpr uint32_t CLUSTER_TEXTEL_SIZE = 64;
	static constexpr uint32_t CLUSTERS_Z = 32;

	static constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 256;

	static constexpr uint32_t CLUSTERED_NUM_BUCKETS = MAX_LIGHTS_PER_CLUSTER / CLUSTERS_Z;

	static constexpr uint32_t CLUSTERS_X_THREADS = 4;
	static constexpr uint32_t CLUSTERS_Y_THREADS = 4;
	static constexpr uint32_t CLUSTERS_Z_THREADS = 4;

	static constexpr uint32_t THREAD_GROUP_SIZE = CLUSTERS_X_THREADS * CLUSTERS_Y_THREADS * CLUSTERS_Z_THREADS;

	struct ClusterAABB {
		rawrbox::Vector4f minBounds = {};
		rawrbox::Vector4f maxBounds = {};
	};

	class ClusteredPlugin : public rawrbox::RenderPlugin {
	protected:
		Diligent::IPipelineState* _clusterBuildingComputeProgram = nullptr;
		Diligent::IShaderResourceBinding* _clusterBuildingComputeBind = nullptr;

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

		virtual void buildBuffers();
		virtual void buildPipelines();

	public:
		static uint32_t CLUSTERS_X;
		static uint32_t CLUSTERS_Y;

		static uint32_t GROUP_SIZE;

		ClusteredPlugin() = default;
		ClusteredPlugin(const ClusteredPlugin&) = default;
		ClusteredPlugin(ClusteredPlugin&&) = delete;
		ClusteredPlugin& operator=(const ClusteredPlugin&) = default;
		ClusteredPlugin& operator=(ClusteredPlugin&&) = delete;
		~ClusteredPlugin() override;

		// UTILS ----
		virtual Diligent::ShaderMacroHelper getClusterMacros();

		virtual Diligent::IBufferView* getClustersBuffer(bool readOnly = true);
		virtual Diligent::IBufferView* getDataGridBuffer(bool readOnly = true);

		virtual void applyPipelineSettings(rawrbox::PipeSettings& settings, bool light = false);
		// ----------

		[[nodiscard]] const std::string getID() const override;

		void initialize(const rawrbox::Vector2i& size) override;
		void resize(const rawrbox::Vector2i& size) override;

		void preRender() override;
	};
} // namespace rawrbox