#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/model/base.hpp>
#include <rawrbox/utils/pack.hpp>

#include <bx/math.h>

#define BGFX_STATE_DEFAULT_3D (0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A)
namespace rawrbox {
	struct Mesh2DShape {
		std::vector<rawrbox::Vector2f> vertex = {};
		std::vector<rawrbox::Vector2f> normal = {};
		std::vector<float> u = {};

		Mesh2DShape() = default;

		std::vector<int> getLineSegments() {
			std::vector<int> segments = {};

			for (size_t i = 0; i < this->vertex.size() - 1; i++) {
				segments.push_back(i);
				segments.push_back(i + 1);
			}

			return segments;
		}
	};

	template <typename M = rawrbox::MaterialBase>
	class Spline : public rawrbox::ModelBase<M> {
		std::vector<std::unique_ptr<rawrbox::BezierCurve>> _curves = {};
		std::unique_ptr<rawrbox::Mesh2DShape> _shape = nullptr;
		float _subDivisions = 8.F;

		void addBezier(std::array<rawrbox::Vector3f, 4> points) {
			this->_curves.push_back(std::make_unique<rawrbox::BezierCurve>(points, this->_subDivisions));
		}

	public:
		explicit Spline(float subDivisions = 8.F) : _subDivisions(subDivisions) {}
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

		virtual void addPoint(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4) {
			this->addBezier({p1, p2, p3, p4});
		}

		virtual void addPoint(const rawrbox::Vector4f& start, const rawrbox::Vector4f& end, float distance = 0.5F) {
			auto axisS = Vector3f::forward().rotate(rawrbox::Vector3f::up(), bx::toRad(start.w)) * distance;
			auto axisE = Vector3f::back().rotate(rawrbox::Vector3f::up(), bx::toRad(end.w)) * distance;

			this->addBezier({start.xyz(), start.xyz() + axisS, end.xyz() + axisE, end.xyz()});
		}

		void generateMesh() {
			if (this->_shape == nullptr) throw std::runtime_error("[RawrBox-Spline] Missing mesh shape!");

			this->_mesh->clear();
			std::vector<int> shapeSegments = this->_shape->getLineSegments();

			for (auto& curve : this->_curves) {
				auto path = curve->generatePath();
				if (path.empty()) return;

				int vertsInShape = this->_shape->vertex.size();
				int segments = path.size() - 1;
				int edgeLoops = path.size();
				int vertCount = vertsInShape * edgeLoops;
				int triCount = shapeSegments.size() * segments;
				int triIndexCount = triCount * 3;

				rawrbox::Mesh mesh;
				std::vector<uint16_t> triangleIndices(triIndexCount);
				std::vector<rawrbox::VertexData> buff(vertCount);

				// Generate all of the vertices and normals
				for (int i = 0; i < path.size(); i++) {
					int offset = i * vertsInShape;
					for (int j = 0; j < vertsInShape; j++) {
						int id = offset + j;

						auto pos = rawrbox::Vector3f(this->_shape->vertex[j].x, this->_shape->vertex[j].y, 0.F);
						auto norm = rawrbox::Vector3f(this->_shape->normal[j].x, this->_shape->normal[j].y, 0.F);
						auto uv = rawrbox::Vector2f(this->_shape->u[j], path[i].vCoordinate);

						buff[id] = rawrbox::VertexData(path[i].LocalToWorld(pos), uv, {rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z), 0}, rawrbox::Colors::White);
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

				mesh.totalIndex = triIndexCount;
				mesh.totalVertex = vertCount;
				mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
				mesh.indices.insert(mesh.indices.end(), triangleIndices.begin(), triangleIndices.end());

				this->_mesh->merge(mesh);
			}
		};

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

			uint64_t flags = BGFX_STATE_DEFAULT_3D /*| this->_mesh->culling*/ | this->_mesh->blending | this->_mesh->depthTest; // TODO: FIX CULLING
			flags |= this->_mesh->lineMode ? BGFX_STATE_PT_LINES : this->_mesh->wireframe ? BGFX_STATE_PT_LINESTRIP
												      : 0;

			bgfx::setState(flags, 0);
			this->_material->postProcess();

			bgfx::discard();
		}
	};
} // namespace rawrbox
