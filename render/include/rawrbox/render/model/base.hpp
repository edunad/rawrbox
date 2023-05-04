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

namespace rawrBox {

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

	template <typename M = rawrBox::MaterialBase>
	class ModelBase {

	protected:
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices

		std::vector<std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>>> _meshes;
		std::array<float, 16> _matrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default

		std::vector<typename M::vertexBufferType> _vertices;
		std::vector<uint16_t> _indices;

		std::unique_ptr<M> _material = std::make_unique<M>();

		rawrBox::Vector3f _scale = {1, 1, 1};
		rawrBox::Vector3f _pos = {};
		rawrBox::Vector3f _angle = {};

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

			this->_meshes.clear();
			this->_vertices.clear();
			this->_indices.clear();
		}

		virtual bool supportsOptimization() { return true; }

		// UTILS -----
		void mergeMeshes(std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> in, std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> other) {
			for (uint16_t i : other->indices)
				in->indices.push_back(static_cast<uint16_t>(in->vertices.size()) + i);

			in->vertices.insert(in->vertices.end(), other->vertices.begin(), other->vertices.end());
			in->totalVertex = static_cast<uint16_t>(in->vertices.size());
			in->totalIndex = static_cast<uint16_t>(in->indices.size());
		}

		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array<typename M::vertexBufferType, 4> buff;
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				buff = {
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
				};
			} else {
				buff = {
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, 0), 1, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, 0), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, 0), 1, 1, cl),
				};
			}

			std::array<uint16_t, 6> inds{
			    0, 1, 2,
			    0, 3, 1};

			mesh->totalVertex = static_cast<uint16_t>(buff.size());
			mesh->totalIndex = static_cast<uint16_t>(inds.size());

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array<typename M::vertexBufferType, 24> buff;

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				// We don't support bones on generated :P, so they just won't be set
				buff = {
				    // Back
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 1, cl),

				    // Front
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

				    // Right
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 0, cl),

				    // Left
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 0, cl),

				    // Top
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

				    // Bottom
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
				    rawrBox::VertexLitData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl)};
			} else {
				buff = {
				    // Back
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), 1, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), 1, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), 0, 1, cl),

				    // Front
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), 1, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), 1, 1, cl),

				    // Right
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), 1, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), 1, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), 0, 0, cl),

				    // Left
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), 1, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), 1, 0, cl),

				    // Top
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), 1, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), 1, 1, cl),

				    // Bottom
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), 1, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), 0, 0, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), 0, 1, cl),
				    rawrBox::VertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), 1, 0, cl)};
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

			mesh->vertices.insert(mesh->vertices.end(), buff.begin(), buff.end());
			for (uint16_t ind : inds)
				mesh->indices.push_back(static_cast<uint16_t>(buff.size()) - ind);

			return mesh;
		}

		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generateAxis(float size, const rawrBox::Vector3f& pos) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			this->mergeMeshes(mesh, generateCube(pos, {size, 0.01F, 0.01F}, Colors::Red));   // x;
			this->mergeMeshes(mesh, generateCube(pos, {0.01F, size, 0.01F}, Colors::Green)); // y;
			this->mergeMeshes(mesh, generateCube(pos, {0.01F, 0.01F, size}, Colors::Blue));  // z;

			mesh->setCulling(0);
			mesh->setTexture(rawrBox::WHITE_TEXTURE);

			return mesh;
		}

		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generateCone(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			return mesh;
		}

		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generateSphere(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White, int ratio = 5) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			/*std::vector<rawrBox::VertexData> buff = {};
			std::vector<uint16_t> inds = {};

			for (size_t ph2 = -90; ph2 < 90; ph2 += ratio) {
				for (size_t th2 = 0; th2 <= 360; th2 += 2 * ratio) {

					double x = std::sin(th2) * std::cos(ph2);
					double y = std::cos(th2) * std::cos(ph2);
					double z = std::sin(ph2);

					buff.emplace_back(rawrBox::Vector3f(x, y, z), rawrBox::PackUtils::packNormal(0, 0, 1), rawrBox::PackUtils::packNormal(0, 0, 1), 0.0f, 0.0f, cl);
				}
			}*/

			return mesh;
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> generateGrid(uint32_t size, const rawrBox::Vector3f& pos, const rawrBox::Colorf& cl = rawrBox::Colors::DarkGray, const rawrBox::Colorf& borderCl = rawrBox::Colors::Transparent) {
			auto mesh = std::make_shared<rawrBox::Mesh<typename M::vertexBufferType>>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::vector<rawrBox::VertexData> buff = {};
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
					buff.emplace_back(rawrBox::Vector3f(pos.x - static_cast<uint32_t>(size / 2), pos.y, pos.z - static_cast<uint32_t>(size / 2)) + rawrBox::Vector3f(x, y, z), 1.0F, 1.0F, col);
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
		virtual const rawrBox::Vector3f& getPos() { return this->_pos; }
		virtual void setPos(const rawrBox::Vector3f& pos) {
			this->_pos = pos;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual const rawrBox::Vector3f& getScale() { return this->_scale; }
		virtual void setScale(const rawrBox::Vector3f& scale) {
			this->_scale = scale;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual const rawrBox::Vector3f& getAngle() { return this->_angle; }
		virtual void setAngle(const rawrBox::Vector3f& ang) {
			this->_angle = ang;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual std::array<float, 16>& getMatrix() {
			return this->_matrix;
		}

		virtual void removeMesh(size_t index) {
			if (index >= this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);
		}

		virtual void addMesh(std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> mesh) {
			this->_meshes.push_back(std::move(mesh));
		}

		virtual std::shared_ptr<rawrBox::Mesh<typename M::vertexBufferType>> getMesh(size_t id = 0) {
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

		virtual void upload() {
			if (bgfx::isValid(this->_vbh) || bgfx::isValid(this->_ibh)) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");
			this->flattenMeshes(); // Merge and optimize meshes for drawing

			// Generate buffers ----
			if (this->_vertices.empty() || this->_indices.empty()) throw std::runtime_error("[RawrBox-ModelBase] Vertices / Indices cannot be empty");
			this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * M::vertexBufferType::vLayout().m_stride), M::vertexBufferType::vLayout());
			this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));
			// -----------------

			this->_material->upload();
			this->_material->registerUniforms();
		}

		virtual void draw(const rawrBox::Vector3f& camPos) {
			if (!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) {
				throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not valid");
			}

			this->_material->preProcess(camPos);
		}
	};
} // namespace rawrBox
