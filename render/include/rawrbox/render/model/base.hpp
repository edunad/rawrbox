#pragma once
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/static.h>
#include <rawrbox/utils/pack.hpp>

#include <bx/math.h>
#include <fmt/printf.h>
#include <vcruntime.h>

#include <array>
#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace rawrBox {

	struct Bone {
		std::string name;
		size_t boneId = 0;

		// Rendering ---
		std::array<float, 16> transformationMtx = {};
		// ----

		// Lookup ----
		std::shared_ptr<Bone> parent;
		std::vector<std::shared_ptr<Bone>> children = {};
		// ----

		Bone(std::string _name) : name(std::move(_name)) {}
	};

	struct Skeleton {
		std::string name;
		std::shared_ptr<Bone> rootBone;

		std::array<float, 16> invTransformationMtx = {};
		Skeleton(std::string _name) : name(std::move(_name)) {}
	};

	class ModelBase {

	protected:
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices

		std::vector<std::shared_ptr<rawrBox::Mesh>> _meshes;
		std::array<float, 16> _matrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default

		std::shared_ptr<rawrBox::MaterialBase> _material;

		std::vector<rawrBox::MeshVertexData> _vertices;
		std::vector<uint16_t> _indices;

		rawrBox::Vector3f _scale = {1, 1, 1};
		rawrBox::Vector3f _pos = {};
		rawrBox::Vector3f _angle = {};

		// SKINNING ----
		std::unordered_map<std::string, std::shared_ptr<Skeleton>> _skeletons = {};
		std::unordered_map<std::string, std::pair<uint8_t, std::array<float, 16>>> _boneMap = {}; // Map for quick lookup
		// --------

		void flattenMeshes(bool quickOptimize = true) {
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

				// Previous mesh available?
				if (quickOptimize) {
					if (mesh != this->_meshes.begin()) {
						auto prevMesh = std::prev(mesh); // Check old meshes

						if ((*prevMesh)->canMerge(*mesh)) {
							fmt::print("Merging {} with {}\n", (*prevMesh)->name, (*mesh)->name);

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

			if (old != this->_meshes.size()) fmt::print("Optimized mesh (Before {} | After {})\n", old, this->_meshes.size());
		}

	public:
		ModelBase(std::shared_ptr<rawrBox::MaterialBase> material) : _material(std::move(material)){};
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

			this->_material = nullptr;
		}

		// UTILS -----
		static std::shared_ptr<rawrBox::Mesh> generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			auto mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array buff{
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, 0), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
			};

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

		static std::shared_ptr<rawrBox::Mesh> generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			auto mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::array buff{
			    // Back
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 1, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(-1, 0, 0), 0, 0, 1, cl),

			    // Front
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

			    // Right
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 1, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, 1), 0, 0, 0, cl),

			    // Left
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(0, 0, -1), 0, 1, 0, cl),

			    // Top
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),

			    // Bottom
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 0, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(-size.x, -size.y, size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 0, 1, cl),
			    rawrBox::MeshVertexData(pos + rawrBox::Vector3f(size.x, -size.y, -size.z), rawrBox::PackUtils::packNormal(1, 0, 0), 0, 1, 0, cl)};

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

		static std::shared_ptr<rawrBox::Mesh> generateAxis(float size, const rawrBox::Vector3f& pos) {
			std::shared_ptr<rawrBox::Mesh> mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			/*auto x = generateCube(pos, {size, 0.01f, 0.01f}, Colors::Red);
			auto y = generateCube(pos, {0.01f, size, 0.01f}, Colors::Green);
			auto z = generateCube(pos, {0.01f, 0.01f, size}, Colors::Blue);

			mesh->vertices.insert(mesh->vertices.end(), x->vertices.begin(), x->vertices.end());
			for (uint16_t i : x->indices)
				mesh->indices.push_back(i);

			mesh->vertices.insert(mesh->vertices.end(), x->vertices.begin(), x->vertices.end());
			for (uint16_t i : y->indices)
				mesh->indices.push_back(i + static_cast<uint16_t>(x->vertices.size()));

			mesh->vertices.insert(mesh->vertices.end(), x->vertices.begin(), x->vertices.end());
			for (uint16_t i : z->indices)
				mesh->indices.push_back(i + static_cast<uint16_t>(y->vertices.size()) + static_cast<uint16_t>(z->vertices.size()));

			mesh->setCulling(0);
			mesh->setTexture(rawrBox::WHITE_TEXTURE);*/

			return mesh;
		}

		static std::shared_ptr<rawrBox::Mesh> generateCone(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh> mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			return mesh;
		}

		static std::shared_ptr<rawrBox::Mesh> generateSphere(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Colorf& cl = rawrBox::Colors::White, int ratio = 5) {
			std::shared_ptr<rawrBox::Mesh> mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			/*std::vector<rawrBox::MeshVertexData> buff = {};
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
		static std::shared_ptr<rawrBox::Mesh> generateGrid(uint32_t size, const rawrBox::Vector3f& pos, const rawrBox::Colorf& cl = rawrBox::Colors::White) {
			std::shared_ptr<rawrBox::Mesh> mesh = std::make_shared<rawrBox::Mesh>();
			bx::mtxTranslate(mesh->vertexPos.data(), pos.x, pos.y, pos.z);

			std::vector<rawrBox::MeshVertexData> buff = {};
			std::vector<uint16_t> inds = {};

			float step = 1.f;
			for (uint32_t j = 0; j <= size; ++j) {
				for (uint32_t i = 0; i <= size; ++i) {
					float x = static_cast<float>(i) / static_cast<float>(step);
					float y = 0;
					float z = static_cast<float>(j) / static_cast<float>(step);
					auto col = cl;

					if (j == 0 || i == 0 || j >= size || i >= size) col = rawrBox::Colors::DarkGray;
					buff.emplace_back(rawrBox::Vector3f(pos.x - static_cast<uint32_t>(size / 2), pos.y, pos.z - static_cast<uint32_t>(size / 2)) + rawrBox::Vector3f(x, y, z), rawrBox::PackUtils::packNormal(0, 0, 1), rawrBox::PackUtils::packNormal(0, 0, 1), 1.0f, 1.0f, col);
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
			if (index < 0 || index > this->_meshes.size()) return;
			this->_meshes.erase(this->_meshes.begin() + index);
		}

		virtual void addMesh(std::shared_ptr<rawrBox::Mesh> mesh) {
			this->_meshes.push_back(std::move(mesh));
		}

		virtual std::shared_ptr<rawrBox::Mesh> getMesh(size_t id = 0) {
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
		// ----

		virtual void upload() {
			if (bgfx::isValid(this->_vbh) || bgfx::isValid(this->_ibh)) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");

			this->flattenMeshes(); // Merge and optimize meshes for drawing
			if (this->_vertices.empty() || this->_indices.empty()) throw std::runtime_error("[RawrBox-ModelBase] Vertices / Indices cannot be empty");

			this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_material->vLayout.m_stride), this->_material->vLayout);
			this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));
			// -----------------

			this->_material->upload();
		}

		virtual void draw(const rawrBox::Vector3f& camPos) {
			if (!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) {
				throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not valid");
			}

			this->_material->preProcess(camPos);
		}
	};
} // namespace rawrBox
