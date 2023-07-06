#pragma once
#include <rawrbox/render/renderers/base.hpp>
#include <rawrbox/render/static.hpp>

#include <stdexcept>

// https://github.com/pezcode/Cluster/blob/master/src/Renderer/Renderer.h

namespace rawrbox {

	struct ClusterVertex {
		// w is padding
		std::array<float, 4> minBounds = {};
		std::array<float, 4> maxBounds = {};

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout layout;
			layout.begin()
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord1, 4, bgfx::AttribType::Float)
			    .end();
			return layout;
		}
	};

	class ClusterUniforms {
	protected:
		bgfx::UniformHandle _clusterSizesVecUniform = BGFX_INVALID_HANDLE;
		bgfx::UniformHandle _zNearFarVecUniform = BGFX_INVALID_HANDLE;

		// dynamic buffers can be created empty
		bgfx::DynamicVertexBufferHandle _clustersBuffer = BGFX_INVALID_HANDLE;
		bgfx::DynamicIndexBufferHandle _lightIndicesBuffer = BGFX_INVALID_HANDLE;
		bgfx::DynamicIndexBufferHandle _lightGridBuffer = BGFX_INVALID_HANDLE;
		bgfx::DynamicIndexBufferHandle _atomicIndexBuffer = BGFX_INVALID_HANDLE;

	public:
		static constexpr uint32_t CLUSTERS_X = 16;
		static constexpr uint32_t CLUSTERS_Y = 8;
		static constexpr uint32_t CLUSTERS_Z = 24;

		// limit number of threads (D3D only allows up to 1024, there might also be shared memory limitations)
		// shader will be run by 6 work groups
		static constexpr uint32_t CLUSTERS_X_THREADS = 16;
		static constexpr uint32_t CLUSTERS_Y_THREADS = 8;
		static constexpr uint32_t CLUSTERS_Z_THREADS = 4;

		static constexpr uint32_t CLUSTER_COUNT = CLUSTERS_X * CLUSTERS_Y * CLUSTERS_Z;

		static constexpr uint32_t MAX_LIGHTS_PER_CLUSTER = 100;

		ClusterUniforms() {
			if (CLUSTERS_Z % CLUSTERS_Z_THREADS != 0) throw std::runtime_error("number of cluster depth slices must be divisible by thread count z-dimension");
		}

		ClusterUniforms(const ClusterUniforms &) = default;
		ClusterUniforms(ClusterUniforms &&) = delete;
		ClusterUniforms &operator=(const ClusterUniforms &) = default;
		ClusterUniforms &operator=(ClusterUniforms &&) = delete;
		~ClusterUniforms() {
			RAWRBOX_DESTROY(this->_clusterSizesVecUniform);
			RAWRBOX_DESTROY(this->_zNearFarVecUniform);

			RAWRBOX_DESTROY(this->_clustersBuffer);
			RAWRBOX_DESTROY(this->_lightIndicesBuffer);
			RAWRBOX_DESTROY(this->_lightGridBuffer);
			RAWRBOX_DESTROY(this->_atomicIndexBuffer);
		}

		void initialize() {
			// Init uniforms
			this->_clusterSizesVecUniform = bgfx::createUniform("u_clusterSizesVec", bgfx::UniformType::Vec4);
			this->_zNearFarVecUniform = bgfx::createUniform("u_zNearFarVec", bgfx::UniformType::Vec4);
			// ----

			// Init buffers
			this->_clustersBuffer =
			    bgfx::createDynamicVertexBuffer(CLUSTER_COUNT, ClusterVertex::vLayout(), BGFX_BUFFER_COMPUTE_READ_WRITE);
			this->_lightIndicesBuffer = bgfx::createDynamicIndexBuffer(CLUSTER_COUNT * MAX_LIGHTS_PER_CLUSTER,
			    BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
			// ----

			// we have to specify the compute buffer format here since we need uvec4
			// not needed for the rest, the default format for vertex/index buffers is vec4/uint
			this->_lightGridBuffer =
			    bgfx::createDynamicIndexBuffer(CLUSTER_COUNT * 4,
				BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32 |
				    BGFX_BUFFER_COMPUTE_FORMAT_32X4 | BGFX_BUFFER_COMPUTE_TYPE_UINT);

			this->_atomicIndexBuffer = bgfx::createDynamicIndexBuffer(1, BGFX_BUFFER_COMPUTE_READ_WRITE | BGFX_BUFFER_INDEX32);
		}

		void setUniforms(const rawrbox::Vector2i &size) const {
			std::array<float, 4> clusterSizesVec = {std::ceil((float)size.x / CLUSTERS_X),
			    std::ceil((float)size.y / CLUSTERS_Y), 0, 0};
			bgfx::setUniform(this->_clusterSizesVecUniform, clusterSizesVec.data());

			std::array<float, 4> zNearFarVec = {rawrbox::MAIN_CAMERA->getZNear(), rawrbox::MAIN_CAMERA->getZFar(), 0, 0};
			bgfx::setUniform(this->_zNearFarVecUniform, zNearFarVec.data());
		}

		void bindBuffers(bool readOnly = true) const {
			// binding ReadWrite in the fragment shader doesn't work with D3D11/12
			bgfx::Access::Enum access = readOnly ? bgfx::Access::Read : bgfx::Access::ReadWrite;

			if (!readOnly) { // read only does not need clusters and atomic index
				bgfx::setBuffer(rawrbox::SAMPLE_CLUSTERS, this->_clustersBuffer, access);
				bgfx::setBuffer(rawrbox::SAMPLE_ATOMIC_INDEX, this->_atomicIndexBuffer, access);
			}

			bgfx::setBuffer(rawrbox::SAMPLE_LIGHTINDICES, this->_lightIndicesBuffer, access);
			bgfx::setBuffer(rawrbox::SAMPLE_LIGHTGRID, this->_lightGridBuffer, access);
		}
	};

	class RendererCluster : public rawrbox::RendererBase {
	protected:
		std::unique_ptr<rawrbox::ClusterUniforms> _uniforms = nullptr;

		bgfx::ProgramHandle _clusterBuildingComputeProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _resetCounterComputeProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _lightCullingComputeProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _lightingProgram = BGFX_INVALID_HANDLE;
		bgfx::ProgramHandle _debugVisProgram = BGFX_INVALID_HANDLE;

		rawrbox::Matrix4x4 _oldProj = {};

	public:
		RendererCluster() = default;
		RendererCluster(const RendererCluster &) = delete;
		RendererCluster(RendererCluster &&) = delete;
		RendererCluster &operator=(const RendererCluster &) = delete;
		RendererCluster &operator=(RendererCluster &&) = delete;
		~RendererCluster() override = default;

		static bool supported();

		void init(const rawrbox::Vector2i &size) override;
		void resize(const rawrbox::Vector2i &size) override;

		void render() override;
		void bindRenderUniforms() override;
	};
} // namespace rawrbox
