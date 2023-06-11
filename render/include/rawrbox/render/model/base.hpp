#pragma once

#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/utils/pack.hpp>

#include <functional>

namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	class ModelBase {

	protected:
		bgfx::DynamicVertexBufferHandle _vbdh = BGFX_INVALID_HANDLE; // Vertices - Dynamic
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE;         // Vertices - Static
		bgfx::DynamicIndexBufferHandle _ibdh = BGFX_INVALID_HANDLE;  // Indices - Dynamic
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;          // Indices - Static

		std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes = {};
		rawrbox::Matrix4x4 _matrix = {};

		std::vector<typename M::vertexBufferType> _vertices = {};
		std::vector<uint16_t> _indices = {};

		std::unique_ptr<M> _material = std::make_unique<M>();

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

		rawrbox::BBOX _bbox = {};

		// BGFX DYNAMIC SUPPORT ---
		bool _isDynamic = false;
		// ----

		bool _canOptimize = true;

		void flattenMeshes() {
			this->_vertices.clear();
			this->_indices.clear();

			// Merge same meshes to reduce calls
			if (this->_canOptimize) {
				size_t old = this->_meshes.size();
				for (auto mesh = this->_meshes.begin(); mesh != this->_meshes.end();) {
					bool merged = false;

					// Previous mesh available?
					if (mesh != this->_meshes.begin()) {
						auto prevMesh = std::prev(mesh); // Check old meshes

						if ((*prevMesh)->canOptimize(**mesh)) {
							(*prevMesh)->merge(**mesh);

							mesh = this->_meshes.erase(mesh);
							merged = true;
						}
					}

					if (!merged) mesh++;
				}

				if (old != this->_meshes.size()) fmt::print("[RawrBox-Model] Optimized mesh for rendering (Before {} | After {})\n", old, this->_meshes.size());
			}
			// ----------------------

			// Flatten meshes for buffers
			for (auto& mesh : this->_meshes) {
				// Fix start index ----
				mesh->baseIndex = static_cast<uint16_t>(this->_indices.size());
				mesh->baseVertex = static_cast<uint16_t>(this->_vertices.size());
				// --------------------

				// Append vertices
				this->_vertices.insert(this->_vertices.end(), mesh->vertices.begin(), mesh->vertices.end());
				this->_indices.insert(this->_indices.end(), mesh->indices.begin(), mesh->indices.end());
				// -----------------
			}
			// --------

			// Sort alpha
			std::sort(this->_meshes.begin(), this->_meshes.end(), [](auto& a, auto& b) {
				return a->blending != BGFX_STATE_BLEND_ALPHA && b->blending == BGFX_STATE_BLEND_ALPHA;
			});
			// --------

			this->updateBuffers();
		}

		virtual void updateBuffers() {
			if (!this->isDynamicBuffer() || !this->isUploaded()) return;

			const bgfx::Memory* vertMem = bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * M::vertexBufferType::vLayout().m_stride);
			const bgfx::Memory* indexMem = bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t));

			bgfx::update(this->_vbdh, 0, vertMem);
			bgfx::update(this->_ibdh, 0, indexMem);
		}

	public:
		ModelBase() = default;
		ModelBase(ModelBase&&) = delete;
		ModelBase& operator=(ModelBase&&) = delete;
		ModelBase(const ModelBase&) = delete;
		ModelBase& operator=(const ModelBase&) = delete;

		virtual ~ModelBase() {
			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);
			RAWRBOX_DESTROY(this->_vbdh);
			RAWRBOX_DESTROY(this->_ibdh);

			this->_meshes.clear();
			this->_vertices.clear();
			this->_indices.clear();
		}

		virtual void setOptimizable(bool status) { this->_canOptimize = status; }

		// UTILS -----
		rawrbox::Mesh<typename M::vertexBufferType> generateLine(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Color& col) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(a);

			std::array<typename M::vertexBufferType, 2> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrbox::VertexLitData(a, {}, {}, col),
				    rawrbox::VertexLitData(b, {}, {}, col),
				};
			} else {
				buff = {
				    rawrbox::VertexData(a, {}, col),
				    rawrbox::VertexData(b, {}, col),
				};
			}

			std::array<uint16_t, 2> inds{0, 1};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = 2;
			mesh.totalIndex = 2;

			mesh.lineMode = true;
			mesh.setOptimizable(false);

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		rawrbox::Mesh<typename M::vertexBufferType> generateTriangle(const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;

			auto center = (a + b + c) / 3;
			mesh.vertexPos.translate(center);

			std::array<typename M::vertexBufferType, 3> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrbox::VertexLitData(a, aUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0}, colA),
				    rawrbox::VertexLitData(b, bUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0}, colB),
				    rawrbox::VertexLitData(c, cUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0}, colC),
				};
			} else {
				buff = {
				    rawrbox::VertexData(a, aUV, colA),
				    rawrbox::VertexData(b, bUV, colB),
				    rawrbox::VertexData(c, cUV, colC),
				};
			}

			std::array<uint16_t, 3> inds{0, 1, 2};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = 3;
			mesh.totalIndex = 3;

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		rawrbox::Mesh<typename M::vertexBufferType> generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			rawrbox::Vector2f hSize = size / 2.F;

			std::array<typename M::vertexBufferType, 4> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, 0), {0, 1}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, 0), {1, 0}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, 0), {0, 0}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, 0), {1, 1}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
				};
			} else {
				buff = {
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, 0), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, 0), {1, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, 0), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, 0), {1, 1}, cl),
				};
			}

			std::array<uint16_t, 6> inds{
			    0, 1, 2,
			    0, 3, 1};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;

			mesh.totalVertex = 4;
			mesh.totalIndex = 6;
			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			// AABB ---
			mesh.bbox.m_min = {-hSize.x, -hSize.y, 0};
			mesh.bbox.m_max = {hSize.x, hSize.y, 0};
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			return mesh;
		}

		rawrbox::Mesh<typename M::vertexBufferType> generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			rawrbox::Vector3f hSize = size / 2.F;

			std::array<typename M::vertexBufferType, 24> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {

				auto nmrlB = rawrbox::PackUtils::packNormal(-1, 0, 0);
				auto nmrlF = rawrbox::PackUtils::packNormal(1, 0, 0);
				auto nmrlR = rawrbox::PackUtils::packNormal(0, 0, -1);
				auto nmrlL = rawrbox::PackUtils::packNormal(0, 0, 1);
				auto nmrlT = rawrbox::PackUtils::packNormal(0, -1, 0);
				auto nmrlBT = rawrbox::PackUtils::packNormal(0, 1, 0);

				buff = {
				    // Back
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlB, nmrlB, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlB, nmrlB, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlB, nmrlB, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlB, nmrlB, 0}, cl),

				    // Front
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 0}, {nmrlF, nmrlF, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 1}, {nmrlF, nmrlF, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 0}, {nmrlF, nmrlF, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 1}, {nmrlF, nmrlF, 0}, cl),

				    // Right
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlR, nmrlR, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {0, 1}, {nmrlR, nmrlR, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlR, nmrlR, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {0, 0}, {nmrlR, nmrlR, 0}, cl),

				    // Left
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlL, nmrlL, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {1, 1}, {nmrlL, nmrlL, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlL, nmrlL, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {1, 0}, {nmrlL, nmrlL, 0}, cl),

				    // Top
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlT, nmrlT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 1}, {nmrlT, nmrlT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlT, nmrlT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 1}, {nmrlT, nmrlT, 0}, cl),

				    // Bottom
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlBT, nmrlBT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 0}, {nmrlBT, nmrlBT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlBT, nmrlBT, 0}, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 0}, {nmrlBT, nmrlBT, 0}, cl)};
			} else {
				buff = {
				    // Back
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {1, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {1, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {0, 1}, cl),

				    // Front
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 1}, cl),

				    // Right
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {0, 0}, cl),

				    // Left
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {1, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {1, 0}, cl),

				    // Top
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 1}, cl),

				    // Bottom
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 0}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 0}, cl)};
			}

			std::array<uint16_t, 36> inds{
			    0, 1, 2,
			    0, 3, 1,

			    4, 6, 5,
			    4, 5, 7,

			    8, 9, 10,
			    8, 11, 9,

			    12, 14, 13,
			    12, 13, 15,

			    16, 18, 17,
			    16, 17, 19,

			    20, 21, 22,
			    20, 23, 21};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh.bbox.m_min = -hSize;
			mesh.bbox.m_max = hSize;
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		rawrbox::Mesh<typename M::vertexBufferType> generateAxis(float size, const rawrbox::Vector3f& pos) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			float hSize = size / 2.F;
			mesh.merge(generateCube(pos, {hSize, 0.01F, 0.01F}, Colors::Red));   // x
			mesh.merge(generateCube(pos, {0.01F, hSize, 0.01F}, Colors::Green)); // y
			mesh.merge(generateCube(pos, {0.01F, 0.01F, hSize}, Colors::Blue));  // z

			// AABB ---
			mesh.bbox.m_min = {-hSize, -hSize, -hSize};
			mesh.bbox.m_max = {hSize, hSize, hSize};
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			mesh.setCulling(0);
			mesh.setTexture(rawrbox::WHITE_TEXTURE.get());

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L687
		// Does not support UV :( / normals
		rawrbox::Mesh<typename M::vertexBufferType> generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, int ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			const float step = rawrbox::pi<float> * 2.0F / ratio;

			const uint32_t numVertices = ratio + 1;
			const uint32_t numIndices = ratio * 6;
			const uint32_t numLineListIndices = ratio * 4;

			std::vector<typename M::vertexBufferType> buff = {};
			buff.resize(numVertices);

			std::vector<uint16_t> index = {};
			index.resize(numIndices + numLineListIndices);

			auto ps = pos;
			ps.y -= size.y / 2.F;

			buff[ratio] = rawrbox::VertexData(ps + rawrbox::Vector3f(0, size.y, 0), {0, 0}, cl);

			for (uint32_t ii = 0; ii < ratio; ++ii) {
				const float angle = step * ii;

				const float angX = std::cos(angle) * size.x;
				const float angZ = std::sin(angle) * size.z;

				buff[ii] = rawrbox::VertexData(ps + rawrbox::Vector3f(angZ, 0.0F, angX), {0, 0}, cl);

				index[ii * 3 + 0] = uint16_t(ratio);
				index[ii * 3 + 1] = uint16_t((ii + 1) % ratio);
				index[ii * 3 + 2] = uint16_t(ii);

				index[ratio * 3 + ii * 3 + 0] = 0;
				index[ratio * 3 + ii * 3 + 1] = uint16_t(ii);
				index[ratio * 3 + ii * 3 + 2] = uint16_t((ii + 1) % ratio);

				index[numIndices + ii * 2 + 0] = uint16_t(ii);
				index[numIndices + ii * 2 + 1] = uint16_t(ratio);

				index[numIndices + ratio * 2 + ii * 2 + 0] = uint16_t(ii);
				index[numIndices + ratio * 2 + ii * 2 + 1] = uint16_t((ii + 1) % ratio);
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(index.size());

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), index.begin(), index.end());

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L750
		// Does not support UV :( / normals
		rawrbox::Mesh<typename M::vertexBufferType> generateCylinder(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, int ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			const float step = rawrbox::pi<float> * 2.0F / ratio;
			const uint32_t numVertices = ratio * 2;
			const uint32_t numIndices = ratio * 12;
			const uint32_t numLineListIndices = ratio * 6;

			std::vector<typename M::vertexBufferType> buff = {};
			buff.resize(numVertices);

			std::vector<uint16_t> index = {};
			index.resize(numIndices + numLineListIndices);

			auto ps = pos;
			ps.y -= size.y;

			for (uint32_t ii = 0; ii < ratio; ++ii) {
				const float angle = step * ii;

				const float angX = std::cos(angle) * size.x;
				const float angZ = std::sin(angle) * size.z;

				buff[ii] = rawrbox::VertexData(ps + rawrbox::Vector3f(angX, size.y, angZ), {0, 0}, cl);
				buff[ii + ratio] = rawrbox::VertexData(ps + rawrbox::Vector3f(angX, 0.0F, angZ), {0, 0}, cl);

				index[ii * 6 + 0] = uint16_t(ii + ratio);
				index[ii * 6 + 1] = uint16_t((ii + 1) % ratio);
				index[ii * 6 + 2] = uint16_t(ii);
				index[ii * 6 + 3] = uint16_t(ii + ratio);
				index[ii * 6 + 4] = uint16_t((ii + 1) % ratio + ratio);
				index[ii * 6 + 5] = uint16_t((ii + 1) % ratio);

				index[ratio * 6 + ii * 6 + 0] = uint16_t(0);
				index[ratio * 6 + ii * 6 + 1] = uint16_t(ii);
				index[ratio * 6 + ii * 6 + 2] = uint16_t((ii + 1) % ratio);
				index[ratio * 6 + ii * 6 + 3] = uint16_t(ratio);
				index[ratio * 6 + ii * 6 + 4] = uint16_t((ii + 1) % ratio + ratio);
				index[ratio * 6 + ii * 6 + 5] = uint16_t(ii + ratio);

				index[numIndices + ii * 2 + 0] = uint16_t(ii);
				index[numIndices + ii * 2 + 1] = uint16_t(ii + ratio);

				index[numIndices + ratio * 2 + ii * 2 + 0] = uint16_t(ii);
				index[numIndices + ratio * 2 + ii * 2 + 1] = uint16_t((ii + 1) % ratio);

				index[numIndices + ratio * 4 + ii * 2 + 0] = uint16_t(ratio + ii);
				index[numIndices + ratio * 4 + ii * 2 + 1] = uint16_t(ratio + (ii + 1) % ratio);
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(index.size());

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), index.begin(), index.end());

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L640
		// Does not support UV :( / normals
		rawrbox::Mesh<typename M::vertexBufferType> generateSphere(const rawrbox::Vector3f& pos, float size, int ratio = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			static const float golden = 1.6180339887F;
			static const float len = std::sqrt(golden * golden + 1.0F);
			static const float ss = 1.0F / len * size;
			static const float ll = ss * golden;

			static const std::array<rawrbox::Vector3f, 32> vv = {
			    rawrbox::Vector3f{-ll, 0.0F, -ss},
			    rawrbox::Vector3f{ll, 0.0F, -ss},
			    rawrbox::Vector3f{ll, 0.0F, ss},
			    rawrbox::Vector3f{-ll, 0.0F, ss},

			    rawrbox::Vector3f{-ss, ll, 0.0F},
			    rawrbox::Vector3f{ss, ll, 0.0F},
			    rawrbox::Vector3f{ss, -ll, 0.0F},
			    rawrbox::Vector3f{-ss, -ll, 0.0F},

			    rawrbox::Vector3f{0.0F, -ss, ll},
			    rawrbox::Vector3f{0.0F, ss, ll},
			    rawrbox::Vector3f{0.0F, ss, -ll},
			    rawrbox::Vector3f{0.0F, -ss, -ll},
			};

			std::vector<typename M::vertexBufferType> buff = {};

			std::function<void(const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint8_t ratio)> triangle;
			triangle = [&triangle, &buff, pos](const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint8_t ratio) {
				if (0 == ratio) {
					buff.push_back(rawrbox::VertexData(pos + _v0, {1, 1}, rawrbox::Colors::White));
					buff.push_back(rawrbox::VertexData(pos + _v2, {1, 0}, rawrbox::Colors::White));
					buff.push_back(rawrbox::VertexData(pos + _v1, {0, 1}, rawrbox::Colors::White));
				} else {
					const rawrbox::Vector3f v01 = (_v0 + _v1).normalized() * _scale;
					const rawrbox::Vector3f v12 = (_v1 + _v2).normalized() * _scale;
					const rawrbox::Vector3f v20 = (_v2 + _v0).normalized() * _scale;

					--ratio;
					triangle(_v0, v01, v20, _scale, ratio);
					triangle(_v1, v12, v01, _scale, ratio);
					triangle(_v2, v20, v12, _scale, ratio);
					triangle(v01, v12, v20, _scale, ratio);
				}
			};

			triangle(vv[0], vv[4], vv[3], size, ratio);
			triangle(vv[0], vv[10], vv[4], size, ratio);
			triangle(vv[4], vv[10], vv[5], size, ratio);
			triangle(vv[5], vv[10], vv[1], size, ratio);
			triangle(vv[5], vv[1], vv[2], size, ratio);
			triangle(vv[5], vv[2], vv[9], size, ratio);
			triangle(vv[5], vv[9], vv[4], size, ratio);
			triangle(vv[3], vv[4], vv[9], size, ratio);

			triangle(vv[0], vv[3], vv[7], size, ratio);
			triangle(vv[0], vv[7], vv[11], size, ratio);
			triangle(vv[11], vv[7], vv[6], size, ratio);
			triangle(vv[11], vv[6], vv[1], size, ratio);
			triangle(vv[1], vv[6], vv[2], size, ratio);
			triangle(vv[2], vv[6], vv[8], size, ratio);
			triangle(vv[8], vv[6], vv[7], size, ratio);
			triangle(vv[8], vv[7], vv[3], size, ratio);

			triangle(vv[0], vv[11], vv[10], size, ratio);
			triangle(vv[1], vv[10], vv[11], size, ratio);
			triangle(vv[2], vv[8], vv[9], size, ratio);
			triangle(vv[3], vv[9], vv[8], size, ratio);

			// ----------

			auto numIndices = static_cast<uint32_t>(buff.size());

			std::vector<uint16_t> trilist = {};
			trilist.resize(numIndices);
			for (uint32_t ii = 0; ii < numIndices; ++ii) {
				trilist[ii] = uint16_t(ii);
			}

			uint32_t numLineListIndices = bgfx::topologyConvert(
			    bgfx::TopologyConvert::TriListToLineList, nullptr, 0, trilist.data(), numIndices, false);

			std::vector<uint16_t> inds = {};
			inds.resize(numLineListIndices * sizeof(uint16_t));

			bgfx::topologyConvert(
			    bgfx::TopologyConvert::TriListToLineList, inds.data(), numLineListIndices * sizeof(uint16_t), trilist.data(), numIndices, false);

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh.bbox.m_min = -_scale / 2.F;
			mesh.bbox.m_max = _scale / 2.F;
			mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
			// -----

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());
			return mesh;
		}

		rawrbox::Mesh<typename M::vertexBufferType> generateMesh(const rawrbox::Vector3f& pos, int size = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;

			std::vector<typename M::vertexBufferType> buff = {};
			auto ps = static_cast<float>(size / 2);

			for (uint32_t y = 0; y < size; y++) {
				for (uint32_t x = 0; x < size; x++) {

					auto xF = static_cast<float>(x);
					auto yF = static_cast<float>(y);

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						buff.push_back(rawrbox::VertexLitData(pos + rawrbox::Vector3f(ps - xF, 0, ps - yF), {(x + 0.5F) / size, (y + 0.5F) / size}, {rawrbox::PackUtils::packNormal(0, 1, 0), 0}, cl));
					} else {
						buff.push_back(rawrbox::VertexData(pos + rawrbox::Vector3f(ps - xF, 0, ps - yF) + pos, {(x + 0.5F) / size, (y + 0.5F) / size}, cl));
					}
				}
			}

			std::vector<uint16_t> inds = {};
			for (uint16_t y = 0; y < (size - 1); y++) {
				uint16_t y_offset = (y * size);

				for (uint16_t x = 0; x < (size - 1); x++) {
					inds.push_back(y_offset + x + 1);
					inds.push_back(y_offset + x + size);
					inds.push_back(y_offset + x);
					inds.push_back(y_offset + x + size + 1);
					inds.push_back(y_offset + x + size);
					inds.push_back(y_offset + x + 1);
				}
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		rawrbox::Mesh<typename M::vertexBufferType> generateGrid(uint32_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::DarkGray, const rawrbox::Colorf& borderCl = rawrbox::Colors::Transparent) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh;
			mesh.vertexPos.translate(pos);

			std::vector<rawrbox::VertexData> buff = {};
			std::vector<uint16_t> inds = {};

			float step = 1.F;
			for (uint32_t j = 0; j <= size; ++j) {
				for (uint32_t i = 0; i <= size; ++i) {
					float x = static_cast<float>(i) / static_cast<float>(step);
					float y = 0;
					float z = static_cast<float>(j) / static_cast<float>(step);
					auto col = cl;

					if (j == 0 || i == 0 || j >= size || i >= size) col = borderCl;
					buff.emplace_back(rawrbox::VertexData{rawrbox::Vector3f(pos.x - static_cast<uint32_t>(size / 2), pos.y, pos.z - static_cast<uint32_t>(size / 2)) + rawrbox::Vector3f(x, y, z), {}, col});
				}
			}

			for (uint32_t j = 0; j < size; ++j) {
				for (uint32_t i = 0; i < size; ++i) {

					uint32_t row1 = j * (size + 1);
					uint32_t row2 = (j + 1) * (size + 1);

					inds.push_back(row1 + i);
					inds.push_back(row1 + i + 1);
					inds.push_back(row1 + i + 1);
					inds.push_back(row2 + i + 1);

					inds.push_back(row2 + i + 1);
					inds.push_back(row2 + i);
					inds.push_back(row2 + i);
					inds.push_back(row1 + i);
				}
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			mesh.lineMode = true;
			return mesh;
		}
		// -------

		// UTIL ---
		[[nodiscard]] virtual const rawrbox::BBOX& getBBOX() const { return this->_bbox; }

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const { return this->_scale; }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const { return this->_angle; }
		virtual void setAngle(const rawrbox::Vector4f& ang) {
			this->_angle = ang;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		virtual void setEulerAngle(const rawrbox::Vector3f& ang) {
			this->_angle = rawrbox::Vector4f::toQuat(ang);
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix() const {
			return this->_matrix;
		}

		[[nodiscard]] virtual const size_t totalMeshes() const {
			return this->_meshes.size();
		}

		[[nodiscard]] virtual const bool empty() const {
			return this->_meshes.empty();
		}

		virtual std::vector<std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>>>& meshes() {
			return this->_meshes;
		}

		[[nodiscard]] virtual const bool isDynamicBuffer() const {
			return this->_isDynamic;
		}

		[[nodiscard]] virtual const bool isUploaded() const {
			if (this->isDynamicBuffer()) return bgfx::isValid(this->_ibdh) && bgfx::isValid(this->_vbdh);
			return bgfx::isValid(this->_ibh) && bgfx::isValid(this->_vbh);
		}

		virtual void removeMeshByName(const std::string& id) {
			auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&id](auto& mesh) { return mesh->getName() == id; });
			if (fnd == this->_meshes.end()) return;

			this->_meshes.erase(fnd);
			if (this->isUploaded() && this->isDynamicBuffer()) this->flattenMeshes(); // Already uploaded? And dynamic? Then update vertices
		}

		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);

			if (this->isUploaded() && this->isDynamicBuffer()) this->flattenMeshes(); // Already uploaded? And dynamic? Then update vertices
		}

		virtual void addMesh(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			this->_bbox.combine(mesh.getBBOX());
			mesh.owner = this;

			this->_meshes.push_back(std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh));
			if (this->isUploaded() && this->isDynamicBuffer()) {
				this->flattenMeshes(); // Already uploaded? And dynamic? Then update vertices
			}
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>* getMeshByName(const std::string& id) {
			auto fnd = std::find_if(this->_meshes.begin(), this->_meshes.end(), [&id](auto& mesh) { return mesh->getName() == id; });
			if (fnd == this->_meshes.end()) return nullptr;

			return (*fnd).get();
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>& getMesh(size_t id = 0) {
			if (!this->hasMesh(id)) throw std::runtime_error(fmt::format("[RawrBox-ModelBase] Mesh {} does not exist", id));
			return *this->_meshes[id];
		}

		virtual bool hasMesh(size_t id) {
			return id < this->_meshes.size();
		}

		virtual void setCulling(uint64_t cull, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setCulling(cull);
			}
		}

		virtual void setWireframe(bool wireframe, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setWireframe(wireframe);
			}
		}

		virtual void setBlend(uint64_t blend, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setBlend(blend);
			}
		}

		virtual void setDepthTest(uint64_t depth, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setDepthTest(depth);
			}
		}

		virtual void setColor(const rawrbox::Color& color, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setColor(color);
			}
		}

		// ----
		virtual void upload(bool dynamic = false) {
			if (this->isUploaded()) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");

			// Generate buffers ----
			this->_isDynamic = dynamic;
			this->flattenMeshes(); // Merge and optimize meshes for drawing

			const bgfx::Memory* vertMem = bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * M::vertexBufferType::vLayout().m_stride);
			const bgfx::Memory* indexMem = bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t));

			if (dynamic) {
				if (this->_vertices.empty() || this->_indices.empty()) {
					this->_vbdh = bgfx::createDynamicVertexBuffer(1, M::vertexBufferType::vLayout(), 0 | BGFX_BUFFER_ALLOW_RESIZE);
					this->_ibdh = bgfx::createDynamicIndexBuffer(1, 0 | BGFX_BUFFER_ALLOW_RESIZE);
				} else {
					this->_vbdh = bgfx::createDynamicVertexBuffer(vertMem, M::vertexBufferType::vLayout(), 0 | BGFX_BUFFER_ALLOW_RESIZE);
					this->_ibdh = bgfx::createDynamicIndexBuffer(indexMem, 0 | BGFX_BUFFER_ALLOW_RESIZE);
				}
			} else {
				if (this->_vertices.empty() || this->_indices.empty()) throw std::runtime_error("[RawrBox-ModelBase] Static buffer cannot contain empty vertices / indices. Use dynamic buffer instead!");

				this->_vbh = bgfx::createVertexBuffer(vertMem, M::vertexBufferType::vLayout());
				this->_ibh = bgfx::createIndexBuffer(indexMem);
			}
			// -----------------

			this->_material->upload();
			this->_material->registerUniforms();
		}

		virtual void draw(const rawrbox::Vector3f& camPos) {
			if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not uploaded");
			this->_material->preProcess(camPos);
		}
	};
} // namespace rawrbox
