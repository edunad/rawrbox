#pragma once
#include <rawrbox/render/model/material/base.hpp>
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/static.h>
#include <rawrbox/utils/pack.hpp>

#include <bx/math.h>
#include <fmt/printf.h>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {

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

		void pushMeshes(bool quickOptimize = true) {
			this->_vertices.clear();
			this->_indices.clear();

			// Merge same meshes to reduce calls
			size_t old = this->_meshes.size();

			for (auto mesh = this->_meshes.begin(); mesh != this->_meshes.end();) {
				bool erased = false;

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
							fmt::print("Merging {} with {}\n", (*mesh)->name, (*mesh)->name);

							(*prevMesh)->totalVertex += (*mesh)->totalVertex;
							(*prevMesh)->totalIndex += (*mesh)->totalIndex;

							mesh = this->_meshes.erase(mesh);
							erased = true;
						}
					}
				}

				if (!erased)
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
					buff.emplace_back(rawrBox::Vector3f(pos.x - size / 2, pos.y, pos.z - size / 2) + rawrBox::Vector3f(x, y, z), rawrBox::PackUtils::packNormal(0, 0, 1), rawrBox::PackUtils::packNormal(0, 0, 1), 1.0f, 1.0f, col);
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
		virtual void setPos(const rawrBox::Vector3f& pos) {
			this->_pos = pos;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

		virtual void setScale(const rawrBox::Vector3f& scale) {
			this->_scale = scale;
			bx::mtxSRT(this->_matrix.data(), this->_scale.x, this->_scale.y, this->_scale.z, bx::toRad(this->_angle.x), bx::toRad(this->_angle.y), bx::toRad(this->_angle.z), this->_pos.x, this->_pos.y, this->_pos.z);
		}

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

			this->pushMeshes();
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
