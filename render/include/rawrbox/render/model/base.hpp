#pragma once
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/model/skeleton.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/utils/pack.hpp>

#include <bx/math.h>
#include <fmt/printf.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace rawrbox {

	template <typename M = rawrbox::MaterialBase>
	class ModelBase {

	protected:
		bgfx::DynamicVertexBufferHandle _vbdh = BGFX_INVALID_HANDLE; // Vertices - Dynamic
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE;         // Vertices - Static
		bgfx::DynamicIndexBufferHandle _ibdh = BGFX_INVALID_HANDLE;  // Indices - Dynamic
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;          // Indices - Static

		std::vector<std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes;
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

		// SKINNING ----
		std::unordered_map<std::string, std::shared_ptr<rawrbox::Skeleton>> _skeletons = {};
		std::unordered_map<std::string, std::shared_ptr<rawrbox::Bone>> _globalBoneMap = {};                                // Map for quick lookup
		std::unordered_map<std::string, std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _animatedMeshes = {}; // Map for quick lookup
		// --------

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

						if ((*prevMesh)->canOptimize(*mesh)) {
							(*prevMesh)->merge(*mesh);

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
			for (auto mesh : this->_meshes) {
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

			this->_animatedMeshes.clear();
			this->_skeletons.clear();
			this->_animatedMeshes.clear();
		}

		virtual void setOptimizable(bool status) { this->_canOptimize = status; }

		// UTILS -----
		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateLine(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Color& col) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			mesh->vertexPos.translate(a);

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

			mesh->baseVertex = 0;
			mesh->baseIndex = 0;
			mesh->totalVertex = 2;
			mesh->totalIndex = 2;

			mesh->lineMode = true;
			mesh->setOptimizable(false);

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateTriangle(const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();

			auto center = (a + b + c) / 3;
			mesh->vertexPos.translate(center);

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

			mesh->baseVertex = 0;
			mesh->baseIndex = 0;
			mesh->totalVertex = 3;
			mesh->totalIndex = 3;

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			mesh->vertexPos.translate(pos);

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

			mesh->baseVertex = 0;
			mesh->baseIndex = 0;

			mesh->totalVertex = 4;
			mesh->totalIndex = 6;
			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

			// AABB ---
			mesh->bbox.m_min = {-hSize.x, -hSize.y, 0};
			mesh->bbox.m_max = {hSize.x, hSize.y, 0};
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			mesh->vertexPos.translate(pos);

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

			mesh->baseVertex = 0;
			mesh->baseIndex = 0;
			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh->bbox.m_min = -hSize;
			mesh->bbox.m_max = hSize;
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateAxis(float size, const rawrbox::Vector3f& pos) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			mesh->vertexPos.translate(pos);

			float hSize = size / 2.F;
			mesh->merge(generateCube(pos, {hSize, 0.01F, 0.01F}, Colors::Red));   // x
			mesh->merge(generateCube(pos, {0.01F, hSize, 0.01F}, Colors::Green)); // y
			mesh->merge(generateCube(pos, {0.01F, 0.01F, hSize}, Colors::Blue));  // z

			// AABB ---
			mesh->bbox.m_min = {-hSize, -hSize, -hSize};
			mesh->bbox.m_max = {hSize, hSize, hSize};
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			mesh->setCulling(0);
			mesh->setTexture(rawrbox::WHITE_TEXTURE);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			throw std::runtime_error("TODO");
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generatePyramid(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			throw std::runtime_error("TODO");
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateCylinder(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			throw std::runtime_error("TODO");
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateSphere(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White, int ratio = 5) {
			throw std::runtime_error("TODO");
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateGrid(uint32_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::DarkGray, const rawrbox::Colorf& borderCl = rawrbox::Colors::Transparent) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			mesh->vertexPos.translate(pos);

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

			mesh->baseVertex = 0;
			mesh->baseIndex = 0;
			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			mesh->indices.insert(mesh->indices.end(), inds.begin(), inds.end());

			mesh->lineMode = true;
			return mesh;
		}
		// -------

		// UTIL ---
		virtual const rawrbox::BBOX& getBBOX() { return this->_bbox; }

		virtual const rawrbox::Vector3f& getPos() { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		virtual const rawrbox::Vector3f& getScale() { return this->_scale; }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		virtual const rawrbox::Vector4f& getAngle() { return this->_angle; }
		virtual void setAngle(const rawrbox::Vector4f& ang) {
			this->_angle = ang;
			this->_matrix.mtxSRT(this->_scale, this->_angle, this->_pos);
		}

		virtual rawrbox::Matrix4x4& getMatrix() {
			return this->_matrix;
		}

		virtual size_t totalMeshes() {
			return this->_meshes.size();
		}

		virtual std::vector<std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>>>& meshes() {
			return this->_meshes;
		}

		virtual bool isDynamicBuffer() {
			return this->_isDynamic;
		}

		virtual bool isUploaded() {
			if (this->isDynamicBuffer()) return bgfx::isValid(this->_ibdh) && bgfx::isValid(this->_vbdh);
			return bgfx::isValid(this->_ibh) && bgfx::isValid(this->_vbh);
		}

		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);

			if (this->isUploaded() && this->isDynamicBuffer()) this->flattenMeshes(); // Already uploaded? And dynamic? Then update vertices
		}

		virtual void addMesh(std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> mesh) {
			this->_bbox.combine(mesh->getBBOX());
			this->_meshes.push_back(std::move(mesh));

			if (this->isUploaded() && this->isDynamicBuffer()) {
				this->flattenMeshes(); // Already uploaded? And dynamic? Then update vertices
			}
		}

		virtual std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> getMesh(size_t id = 0) {
			if (id >= this->_meshes.size()) throw std::runtime_error(fmt::format("[RawrBox-ModelBase] Mesh {} does not exist", id));
			return this->_meshes[id];
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
				if (this->_vertices.empty() || this->_indices.empty()) throw std::runtime_error("[RawrBox-ModelBase] Static buffer cannot contain empty vertices / indices");

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
