#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/model/base.hpp>

namespace rawrbox {
	template <typename M = rawrbox::MaterialBase>
	class Spline : public rawrbox::ModelBase<M> {
		rawrbox::BezierCurve _curve = {};
		std::unique_ptr<rawrbox::Mesh<typename M::vertexBufferType>> _template = nullptr;

		std::vector<int> getLineSegments() {
			std::vector<int> segments = {};
			if (this->_template == nullptr) return segments;
			for (size_t i = 0; i < this->_template->vertices.size(); i++) {
				segments.push_back(i);
				segments.push_back(i + 1);
			}

			return segments;
		}

		void generateMesh() {
			if (this->_template == nullptr) throw std::runtime_error("[RawrBox-Spline] Missing mesh template!");

			auto paths = this->_curve.generatePath(5.F);
			if (paths.empty()) return;

			auto shapeSegments = this->getLineSegments();
			int vertsInShape = this->_template->vertices.size();

			int segments = paths.size() - 1;
			int edgeLoops = paths.size();
			int vertCount = vertsInShape * edgeLoops;
			int triCount = shapeSegments.size() * segments * 2;
			int triIndexCount = triCount * 3;

			std::vector<uint16_t> indices(triIndexCount);
			std::vector<typename M::vertexBufferType> buff(vertCount);

			// Generate all of the vertices and normals
			for (int i = 0; i < paths.size(); i++) {
				int offset = i * vertsInShape;
				for (int j = 0; j < vertsInShape; j++) {
					int id = offset + j;
					auto vertex = this->_template->vertices[j];

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						auto normal = paths[i].LocalToWorldDirection(vertex.normals[j]);
						buff[id] = rawrbox::VertexLitData(paths[i].LocalToWorld({vertex.position[0], vertex.position[1], vertex.position[2]}), {vertex.uv[0], paths[i].vCoordinate}, {rawrbox::PackUtils::packNormal(normal.x, normal.y, normal.z), 0}, rawrbox::Colors::White);
					} else {
						buff[id] = rawrbox::VertexData(paths[i].LocalToWorld({vertex.position[0], vertex.position[1], vertex.position[2]}), {vertex.uv[0], paths[i].vCoordinate}, rawrbox::Colors::White);
					}

					/*vertices[id] = paths[i].LocalToWorld(vertex.position);
					// normals[id] = paths[i].LocalToWorldDirection(shape.normals[j]);
					uvs[id] = rawrbox::Vector2f();*/
				}
			}

			// Generate all of the triangles
			int ti = 0;
			for (int i = 0; i < segments; i++) {
				int offset = i * vertsInShape;
				for (int l = 0; l < shapeSegments.size(); l += 2) {
					int a = offset + shapeSegments[l];
					int b = offset + shapeSegments[l] + vertsInShape;
					int c = offset + shapeSegments[l + 1];
					int d = offset + shapeSegments[l + 1] + vertsInShape;

					indices[ti++] = a; // 0
					indices[ti++] = b; // 1
					indices[ti++] = c; // 2

					indices[ti++] = a; // 0
					indices[ti++] = d; // 3
					indices[ti++] = b; // 1
				}
			}

			this->_mesh->clear();

			this->_mesh->baseVertex = 0;
			this->_mesh->baseIndex = 0;
			this->_mesh->totalVertex = vertCount;
			this->_mesh->totalIndex = triIndexCount;

			this->_mesh->vertices.insert(this->_mesh->vertices.end(), buff.begin(), buff.end());
			this->_mesh->indices.insert(this->_mesh->indices.end(), indices.begin(), indices.end());
		};

	public:
		explicit Spline() = default;
		Spline(const Spline&) = delete;
		Spline(Spline&&) = delete;
		Spline& operator=(const Spline&) = delete;
		Spline& operator=(Spline&&) = delete;
		~Spline() override = default;

		virtual void setTemplate(rawrbox::Mesh<typename M::vertexBufferType> mesh) {
			if (mesh.empty()) throw std::runtime_error("[RawrBox-Spline] Invalid mesh! Missing vertices / indices!");

			this->_template = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh);
			this->_mesh = std::make_unique<rawrbox::Mesh<typename M::vertexBufferType>>(mesh); // Copy over settings, but we remove the vertices anyways
			this->_mesh->clear();
		}

		virtual void setPoints(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4) {
			this->_curve.setPoints({p1, p2, p3, p4});
			this->generateMesh();
		}

		void draw() override {
			ModelBase<M>::draw();
			this->_material->process(*this->_mesh); // Set atlas

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh);
				bgfx::setIndexBuffer(this->_ibdh);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh);
				bgfx::setIndexBuffer(this->_ibh);
			}

			bgfx::setTransform((this->getMatrix()).data());
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_CULL_CW, 0);
			this->_material->postProcess();
		}
	};
} // namespace rawrbox
