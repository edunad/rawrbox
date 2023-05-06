#pragma once
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>
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

	struct Bone {
		std::string name;
		uint8_t boneId = 0;

		// Rendering ---
		std::array<float, 16> transformationMtx = {};
		std::array<float, 16> offsetMtx = {};
		// ----

		// Lookup ----
		Skeleton* owner = nullptr;

		std::shared_ptr<Bone> parent;
		std::vector<std::shared_ptr<Bone>> children = {};
		// ----

		explicit Bone(std::string _name) : name(std::move(_name)) {}
	};

	struct Skeleton {
		uint8_t boneIndex = 0;

		std::string name;
		std::shared_ptr<Bone> rootBone;

		std::array<float, 16> invTransformationMtx = {};

		explicit Skeleton(std::string _name) : name(std::move(_name)) {}
	};

	template <typename M = rawrbox::MaterialBase>
	class ModelBase {

	protected:
		bgfx::DynamicVertexBufferHandle _vbdh = BGFX_INVALID_HANDLE; // Vertices - Dynamic
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE;         // Vertices - Static
		bgfx::DynamicIndexBufferHandle _ibdh = BGFX_INVALID_HANDLE;  // Indices - Dynamic
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;          // Indices - Static

		std::vector<std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>>> _meshes;
		std::array<float, 16> _matrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default

		std::vector<typename M::vertexBufferType> _vertices = {};
		std::vector<uint16_t> _indices = {};

		std::unique_ptr<M> _material = std::make_unique<M>();

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector3f _angle = {};

		rawrbox::BBOX _bbox = {};

		// BGFX DYNAMIC SUPPORT ---
		bool _isDynamic = false;
		// ----

		// SKINNING ----
		std::unordered_map<std::string, std::shared_ptr<Skeleton>> _skeletons = {};
		std::unordered_map<std::string, std::shared_ptr<Bone>> _globalBoneMap = {}; // Map for quick lookup
		// --------

		void flattenMeshes() {
			this->_vertices.clear();
			this->_indices.clear();

			// Merge same meshes to reduce calls
			size_t old = this->_meshes.size();

			for (auto mesh = this->_meshes.begin(); mesh != this->_meshes.end();) {
				bool merged = false;

				// Fix start index ----
				(*mesh)->baseIndex = static_cast<uint16_t>(this->_indices.size());
				// --------------------

				// Append vertices and indices
				this->_vertices.insert(this->_vertices.end(), (*mesh)->vertices.begin(), (*mesh)->vertices.end());

				auto pos = static_cast<uint16_t>(this->_vertices.size());
				for (auto& in : (*mesh)->indices)
					this->_indices.push_back(pos - in);
				// ------

				if (this->supportsOptimization()) {
					// Previous mesh available?
					if (mesh != this->_meshes.begin()) {
						auto prevMesh = std::prev(mesh); // Check old meshes

						if ((*prevMesh)->canMerge(*mesh)) {
							(*prevMesh)->totalVertex += (*mesh)->totalVertex;
							(*prevMesh)->totalIndex += (*mesh)->totalIndex;

							mesh = this->_meshes.erase(mesh);
							merged = true;
						}
					}
				}

				if (!merged)
					mesh++;
			}

			if (old != this->_meshes.size()) fmt::print("[RawrBox-Model] Optimized mesh (Before {} | After {})\n", old, this->_meshes.size());
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

		virtual bool supportsOptimization() { return true; }

		// UTILS -----
		void mergeMeshes(std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> in, std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> other) {
			for (uint16_t i : other->indices)
				in->indices.push_back(static_cast<uint16_t>(in->vertices.size()) + i);

			in->vertices.insert(in->vertices.end(), other->vertices.begin(), other->vertices.end());
			in->totalVertex = static_cast<uint16_t>(in->vertices.size());
			in->totalIndex = static_cast<uint16_t>(in->indices.size());
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateTriangle(const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();

			auto center = (a + b + c) / 3;
			bx::mtxTranslate(mesh->vertexPos.data(), center.x, center.y, center.z);

			std::array<typename M::vertexBufferType, 3> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrbox::VertexLitData(a, rawrbox::PackUtils::packNormal(1, 0, 0), 0, aUV.x, aUV.y, colA),
				    rawrbox::VertexLitData(b, rawrbox::PackUtils::packNormal(1, 0, 0), 0, bUV.x, bUV.y, colB),
				    rawrbox::VertexLitData(c, rawrbox::PackUtils::packNormal(1, 0, 0), 0, cUV.x, cUV.y, colC),
				};
			} else {
				buff = {
				    rawrbox::VertexData(a, aUV.x, aUV.y, colA),
				    rawrbox::VertexData(b, bUV.x, bUV.y, colB),
				    rawrbox::VertexData(c, cUV.x, cUV.y, colC),
				};
			}

			std::array<uint16_t, 3> inds{0, 1, 2};

			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array<typename M::vertexBufferType, 4> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, 0), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, 0), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, 0), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, 0), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
				};
			} else {
				buff = {
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, 0), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, 0), 1, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, 0), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, 0), 1, 1, cl),
				};
			}

			std::array<uint16_t, 6> inds{
			    0, 1, 2,
			    0, 3, 1};

			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh->bbox.m_min = {-size.x, -size.y, 0};
			mesh->bbox.m_max = {size.x, size.y, 0};
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array<typename M::vertexBufferType, 24> buff;

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				// We don't support bones on generated :P, so they just won't be set
				buff = {
				    // Back
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, size.z), rawrbox::PackUtils::packNormal(-1, 0, 0), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(-1, 0, 0), 0, 1, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), rawrbox::PackUtils::packNormal(-1, 0, 0), 0, 1, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(-1, 0, 0), 0, 0, 1, cl),

				    // Front
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

				    // Right
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, size.z), rawrbox::PackUtils::packNormal(0, 0, 1), 0, 1, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(0, 0, 1), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(0, 0, 1), 0, 1, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(0, 0, 1), 0, 0, 0, cl),

				    // Left
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), rawrbox::PackUtils::packNormal(0, 0, -1), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(0, 0, -1), 0, 1, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(0, 0, -1), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(0, 0, -1), 0, 1, 0, cl),

				    // Top
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

				    // Bottom
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrbox::VertexLitData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), rawrbox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl)};
			} else {
				buff = {
				    // Back
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, size.z), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), 1, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), 1, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), 0, 1, cl),

				    // Front
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), 1, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), 1, 1, cl),

				    // Right
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, size.z), 1, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), 1, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), 0, 0, cl),

				    // Left
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), 1, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), 1, 0, cl),

				    // Top
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, size.z), 1, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, -size.z), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, size.y, size.z), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, size.y, -size.z), 1, 1, cl),

				    // Bottom
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, size.z), 1, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, -size.z), 0, 0, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-size.x, -size.y, size.z), 0, 1, cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(size.x, -size.y, -size.z), 1, 0, cl)};
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

			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh->bbox.m_min = -size;
			mesh->bbox.m_max = size;
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateAxis(float size, const rawrbox::Vector3f& pos) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			this->mergeMeshes(mesh, generateCube(pos, {size, 0.01F, 0.01F}, Colors::Red));   // x;
			this->mergeMeshes(mesh, generateCube(pos, {0.01F, size, 0.01F}, Colors::Green)); // y;
			this->mergeMeshes(mesh, generateCube(pos, {0.01F, 0.01F, size}, Colors::Blue));  // z;

			// AABB ---
			mesh->bbox.m_min = {-size, -size, -size};
			mesh->bbox.m_max = {size, size, size};
			mesh->bbox.m_size = mesh->bbox.m_min.abs() + mesh->bbox.m_max.abs();
			// -----

			mesh->setCulling(0);
			mesh->setTexture(rawrbox::WHITE_TEXTURE);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			return mesh;
		}

		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateSphere(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White, int ratio = 5) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			/*std::vector<rawrbox::VertexData> buff = {};
			std::vector<uint16_t> inds = {};

			for (size_t ph2 = -90; ph2 < 90; ph2 += ratio) {
				for (size_t th2 = 0; th2 <= 360; th2 += 2 * ratio) {

					double x = std::sin(th2) * std::cos(ph2);
					double y = std::cos(th2) * std::cos(ph2);
					double z = std::sin(ph2);

					buff.emplace_back(rawrbox::Vector3f(x, y, z), rawrbox::PackUtils::packNormal(0, 0, 1), rawrbox::PackUtils::packNormal(0, 0, 1), 0.0f, 0.0f, cl);
				}
			}*/

			return mesh;
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		std::shared_ptr<rawrbox::Mesh<typename M::vertexBufferType>> generateGrid(uint32_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::DarkGray, const rawrbox::Colorf& borderCl = rawrbox::Colors::Transparent) {
			auto mesh = std::make_shared<rawrbox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

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
					// No need to support ligth, it's a grid
					buff.emplace_back(rawrbox::Vector3f(pos.x - static_cast<uint32_t>(size / 2), pos.y, pos.z - static_cast<uint32_t>(size / 2)) + rawrbox::Vector3f(x, y, z), 1.0F, 1.0F, col);
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

			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			mesh->wireframe = true;
			return mesh;
		}
		// -------

		// UTIL ---
		virtual const rawrbox::BBOX& getBBOX() { return this->_bbox; }

		virtual const rawrbox::Vector3f& getPos() { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual const rawrbox::Vector3f& getScale() { return this->_scale; }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual const rawrbox::Vector3f& getAngle() { return this->_angle; }
		virtual void setAngle(const rawrbox::Vector3f& ang) {
			this->_angle = ang;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual std::array<float, 16>& getMatrix() {
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
			if (!this->isUploaded()) throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not valid");
			this->_material->preProcess(camPos);
		}
	};
} // namespace rawrbox
