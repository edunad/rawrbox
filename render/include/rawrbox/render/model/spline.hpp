#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/model/base.hpp>

namespace rawrbox {
	struct Mesh2DShape {
		std::vector<rawrbox::Vector2f> position = {};
		std::vector<rawrbox::Vector2f> normal = {};
		std::vector<float> u = {};

		Mesh2DShape() = default;

		std::vector<int> getLineSegments() {
			std::vector<int> segments = {};
			for (size_t i = 0; i < this->position.size() - 1; i++) {
				segments.push_back(i);
				segments.push_back(i + 1);
			}

			return segments;
		}
	};

	template <typename M = rawrbox::MaterialBase>
	class Spline : public rawrbox::ModelBase<M> {
		rawrbox::BezierCurve _curve = {};
		std::unique_ptr<rawrbox::Mesh2DShape> _shape = nullptr;

		void generateMesh() {
			if (this->_shape == nullptr) throw std::runtime_error("[RawrBox-Spline] Missing mesh shape!");

			auto path = this->_curve.generatePath(10.F);
			if (path.empty()) return;

			int vertsInShape = this->_shape->position.size();
			std::vector<int> shapeSegments = this->_shape->getLineSegments();

			int segments = path.size() - 1;
			int edgeLoops = path.size();
			int vertCount = vertsInShape * edgeLoops;
			int triCount = shapeSegments.size() * segments * 2;
			int triIndexCount = triCount * 3;

			std::vector<uint16_t> triangleIndices(triIndexCount);
			std::vector<typename M::vertexBufferType> buff(vertCount);

			// Generate all of the vertices and normals
			for (int i = 0; i < path.size(); i++) {
				int offset = i * vertsInShape;
				for (int j = 0; j < vertsInShape; j++) {
					int id = offset + j;

					auto pos = rawrbox::Vector3f(this->_shape->position[j].x, this->_shape->position[j].y, 0.F);
					auto norm = rawrbox::Vector3f(this->_shape->normal[j].x, this->_shape->normal[j].y, 0.F);
					auto uv = rawrbox::Vector2f(this->_shape->u[j], path[i].vCoordinate);

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						buff[id] = rawrbox::VertexLitData(path[i].LocalToWorld(pos), uv, {rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z), 0}, rawrbox::Colors::White);
					} else {
						buff[id] = rawrbox::VertexData(path[i].LocalToWorld(pos), uv, rawrbox::Colors::White);
					}
				}
			}

			int ti = 0;
			for (int i = 0; i < segments; i++) {
				int offset = i * vertsInShape;
				for (int l = 0; l < shapeSegments.size(); l += 2) {
					int a = offset + shapeSegments[l];
					int b = offset + shapeSegments[l] + vertsInShape;
					int c = offset + shapeSegments[l + 1] + vertsInShape;
					int d = offset + shapeSegments[l + 1];

					triangleIndices[ti++] = a; // 0
					triangleIndices[ti++] = b; // 1
					triangleIndices[ti++] = c; // 2

					triangleIndices[ti++] = d; // 3
					triangleIndices[ti++] = c; // 2
					triangleIndices[ti++] = a; // 0
				}
			}

			this->_mesh->clear();

			this->_mesh->baseVertex = 0;
			this->_mesh->baseIndex = 0;
			this->_mesh->totalVertex = vertCount;
			this->_mesh->totalIndex = triIndexCount;

			this->_mesh->vertices.insert(this->_mesh->vertices.end(), buff.begin(), buff.end());
			this->_mesh->indices.insert(this->_mesh->indices.end(), triangleIndices.begin(), triangleIndices.end());
		};

	public:
		explicit Spline() = default;
		Spline(const Spline&) = delete;
		Spline(Spline&&) = delete;
		Spline& operator=(const Spline&) = delete;
		Spline& operator=(Spline&&) = delete;
		~Spline() override = default;

		virtual void setExtrudeVerts(rawrbox::Mesh2DShape shape) {
			this->_shape = std::make_unique<rawrbox::Mesh2DShape>(shape);
		}

		virtual void setTexture(rawrbox::TextureBase* texture) {
			this->_mesh->setTexture(texture);
		}

		virtual void setPoints(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4) {
			this->_curve.setPoints({p1, p2, p3, p4});
			this->generateMesh();
		}

		void draw() override {
			ModelBase<M>::draw();
			this->_material->process(*this->_mesh);

			if (this->isDynamicBuffer()) {
				bgfx::setVertexBuffer(0, this->_vbdh);
				bgfx::setIndexBuffer(this->_ibdh);
			} else {
				bgfx::setVertexBuffer(0, this->_vbh);
				bgfx::setIndexBuffer(this->_ibh);
			}

			bgfx::setTransform((this->getMatrix()).data());
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS, 0);

			this->_material->postProcess();
		}
	};
} // namespace rawrbox
