#pragma once
#include <rawrbox/render/model/mesh.hpp>
#include <rawrbox/render/static.h>
#include <rawrbox/render/texture/flat.h>
#include <rawrbox/render/util/uniforms.hpp>

#include <array>
#include <memory>
#include <vector>

namespace rawrBox {
	template <typename T>
	class ModelBase {

	protected:
		bgfx::VertexLayout _vLayout;

		bgfx::ProgramHandle _program = BGFX_INVALID_HANDLE;
		bgfx::VertexBufferHandle _vbh = BGFX_INVALID_HANDLE; // Vertices
		bgfx::IndexBufferHandle _ibh = BGFX_INVALID_HANDLE;  // Indices

		bgfx::UniformHandle _viewPos = BGFX_INVALID_HANDLE;

		std::vector<std::shared_ptr<rawrBox::Mesh<T>>> _meshes;
		std::array<float, 16> _matrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default

		std::vector<T> _vertices;
		std::vector<uint16_t> _indices;

		uint64_t _cull = BGFX_STATE_CULL_CW;
		bool _fullbright = false;

		void internalUpdate() {
			this->_vertices.clear();
			this->_indices.clear();

			for (auto m : this->_meshes) {
				auto& data = m->getData();

				// Fix textures & base index
				data->baseIndex = static_cast<uint16_t>(this->_indices.size());
				// ----

				// Append vertices and indices
				this->_vertices.insert(this->_vertices.end(), data->vertices.begin(), data->vertices.end());
				auto pos = static_cast<uint16_t>(this->_vertices.size());
				for (auto& in : data->indices)
					this->_indices.push_back(pos - in);
				// ---
			}
		}

	public:
		ModelBase() = default;
		virtual ~ModelBase() {
			RAWRBOX_DESTROY(this->_vbh);
			RAWRBOX_DESTROY(this->_ibh);
			RAWRBOX_DESTROY(this->_program);

			RAWRBOX_DESTROY(this->_viewPos);

			this->_meshes.clear();
			this->_vertices.clear();
			this->_indices.clear();
		}

		// UTIL ---
		virtual void setPos(const rawrBox::Vector3f& pos) {
			float p[16];
			bx::mtxTranslate(p, pos.x, pos.y, pos.z);
			bx::mtxMul(this->_matrix.data(), this->_matrix.data(), p);
		}

		virtual void setScale(const rawrBox::Vector3f& scale) {
			float p[16];
			bx::mtxScale(p, scale.x, scale.y, scale.z);
			bx::mtxMul(this->_matrix.data(), this->_matrix.data(), p);
		}

		virtual void setAngle(const rawrBox::Vector3f& ang) {
			float p[16];
			bx::mtxRotateXYZ(p, ang.x, ang.y, ang.z);
			bx::mtxMul(this->_matrix.data(), this->_matrix.data(), p);
		}

		virtual void setMatrix(const std::array<float, 16>& matrix) {
			this->_matrix = matrix;
		}

		virtual std::array<float, 16>& getMatrix() {
			return this->_matrix;
		}

		virtual void removeMesh(size_t index) {
			if (index < 0 || index > this->_meshes.size()) return;

			this->_meshes.erase(this->_meshes.begin() + index);
			this->internalUpdate();
		}

		virtual void addMesh(const std::shared_ptr<rawrBox::Mesh<T>>& mesh) {
			this->_meshes.push_back(std::move(mesh));
			this->internalUpdate();
		}

		virtual const std::shared_ptr<rawrBox::Mesh<T>>& getMesh(size_t id = 0) {
			return this->_meshes[id];
		}

		virtual void setWireframe(bool wireframe, int id = -1) {
			for (size_t i = 0; i < this->_meshes.size(); i++) {
				if (id != -1 && i != id) continue;
				this->_meshes[i]->setWireframe(wireframe);
			}
		}

		virtual void setCulling(uint64_t cull) {
			this->_cull = cull;
		}

		virtual void setFullbright(bool b) {
			this->_fullbright = b;
		}
		// ----

		virtual void upload() {
			if (bgfx::isValid(this->_vbh) || bgfx::isValid(this->_ibh)) throw std::runtime_error("[RawrBox-ModelBase] Upload called twice");
			if (this->_vertices.empty() || this->_indices.empty()) throw std::runtime_error("[RawrBox-ModelBase] Vertices / Indices cannot be empty");

			this->_vbh = bgfx::createVertexBuffer(bgfx::makeRef(this->_vertices.data(), static_cast<uint32_t>(this->_vertices.size()) * this->_vLayout.m_stride), this->_vLayout);
			this->_ibh = bgfx::createIndexBuffer(bgfx::makeRef(this->_indices.data(), static_cast<uint32_t>(this->_indices.size()) * sizeof(uint16_t)));
			// -----------------

			this->_viewPos = bgfx::createUniform("u_viewPos", bgfx::UniformType::Vec4, 3);
		}

		virtual void draw(const rawrBox::Vector3f& camPos) {
			if (!bgfx::isValid(this->_vbh) || !bgfx::isValid(this->_ibh)) {
				throw std::runtime_error("[RawrBox-Model] Failed to render model, vertex / index buffer is not valid");
			}

			// Setup camera view pos ---
			rawrBox::UniformUtils::setUniform(this->_viewPos, camPos);
			// -----
		}
	};
} // namespace rawrBox
