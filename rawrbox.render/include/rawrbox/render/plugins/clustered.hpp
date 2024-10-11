#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/plugins/base.hpp>

#include <RefCntAutoPtr.hpp>
#include <ShaderMacroHelper.hpp>

#include <Buffer.h>
#include <PipelineState.h>

namespace rawrbox {
	struct ClusterAABB {
		rawrbox::Vector4f minBounds = {};
		rawrbox::Vector4f maxBounds = {};
	};

	class ClusteredPlugin : public rawrbox::RenderPlugin {
	protected:
		Diligent::IPipelineState* _clusterBuildingComputeProgram = nullptr;
		Diligent::IPipelineState* _cullingComputeProgram = nullptr;
		Diligent::IPipelineState* _cullingResetProgram = nullptr;

		Diligent::DispatchComputeAttribs _dispatch = {};

		// BUFFERS ---
		Diligent::RefCntAutoPtr<Diligent::IBuffer> _clusterBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _clusterBufferRead;

		Diligent::RefCntAutoPtr<Diligent::IBuffer> _dataGridBuffer;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferWrite;
		Diligent::RefCntAutoPtr<Diligent::IBufferView> _dataGridBufferRead;
		// -----------

		// SIGNATURE ---
		Diligent::RefCntAutoPtr<Diligent::IPipelineResourceSignature> _signature;
		Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> _signatureBind;
		// --------------

		rawrbox::Matrix4x4 _oldProj = {};

		virtual void buildBuffers();
		virtual void buildSignatures();
		virtual void buildPipelines();

	public:
		static uint32_t CLUSTERS_X;
		static uint32_t CLUSTERS_Y;

		static uint32_t CLUSTERS_GROUP_SIZE;

		ClusteredPlugin() = default;
		ClusteredPlugin(const ClusteredPlugin&) = delete;
		ClusteredPlugin(ClusteredPlugin&&) = delete;
		ClusteredPlugin& operator=(const ClusteredPlugin&) = delete;
		ClusteredPlugin& operator=(ClusteredPlugin&&) = delete;
		~ClusteredPlugin() override;

		// UTILS ----
		virtual Diligent::ShaderMacroHelper getClusterMacros();

		virtual Diligent::IBufferView* getClustersBuffer(bool readOnly = true);
		virtual Diligent::IBufferView* getDataGridBuffer(bool readOnly = true);
		// ----------

		void initialize(const rawrbox::Vector2u& size) override;
		void resize(const rawrbox::Vector2u& size) override;
		void upload() override;

		void signatures(std::vector<Diligent::PipelineResourceDesc>& sig) override;
		void bindStatic(Diligent::IPipelineResourceSignature& sig) override;

		void preRender(const rawrbox::CameraBase& camera) override;

		std::string getID() override;
	};
} // namespace rawrbox
