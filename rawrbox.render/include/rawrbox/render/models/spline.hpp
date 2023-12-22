#pragma once

#include <rawrbox/math/bezier_curve.hpp>
#include <rawrbox/render/models/base.hpp>
#include <rawrbox/utils/pack.hpp>

namespace rawrbox {
	struct Mesh2DShape {
		std::vector<rawrbox::Vector2f> vertex = {};
		std::vector<rawrbox::Vector2f> normal = {};
		std::vector<float> u = {};

		Mesh2DShape() = default;

		std::vector<int> getLineSegments() {
			std::vector<int> segments = {};

			for (size_t i = 0; i < this->vertex.size() - 1; i++) {
				segments.push_back(static_cast<int>(i));
				segments.push_back(static_cast<int>(i) + 1);
			}

			return segments;
		}
	};

	template <typename M = rawrbox::MaterialUnlit>
	class Spline : public rawrbox::ModelBase<M> {
	protected:
		std::vector<std::unique_ptr<rawrbox::BezierCurve>> _curves = {};
		std::unique_ptr<rawrbox::Mesh2DShape> _shape = nullptr;
		float _subDivisions = 8.F;

		virtual void addBezier(std::array<rawrbox::Vector3f, 4> points) {
			this->_curves.push_back(std::make_unique<rawrbox::BezierCurve>(points, this->_subDivisions));
		}

	public:
		explicit Spline(float subDivisions = 8.F) : _subDivisions(subDivisions) {
			this->_mesh->setCulling(Diligent::CULL_MODE_NONE);
		}

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

		virtual void setColor(const rawrbox::Color& col) {
			this->_mesh->setColor(col);
		}

		virtual rawrbox::Mesh<typename M::vertexBufferType>& getMesh() {
			return *this->_mesh.get();
		}

		virtual const std::vector<std::unique_ptr<rawrbox::BezierCurve>>& getCurves() {
			return this->_curves;
		}

		virtual void addPoint(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4) {
			this->addBezier({p1, p2, p3, p4});
		}

		virtual void addPoint(const rawrbox::Vector4f& start, const rawrbox::Vector4f& end, float distance = 0.5F) {
			auto axisS = Vector3f::forward().rotateAroundOrigin(rawrbox::Vector3f::up(), rawrbox::MathUtils::toRad(start.w)) * distance;
			auto axisE = (-Vector3f::forward()).rotateAroundOrigin(rawrbox::Vector3f::up(), rawrbox::MathUtils::toRad(end.w)) * distance;

			this->addBezier({start.xyz(), start.xyz() + axisS, end.xyz() + axisE, end.xyz()});
		}

		virtual void generateMesh() {
			if (this->_shape == nullptr) throw std::runtime_error("[RawrBox-Spline] Missing mesh shape!");

			this->_mesh->clear();
			std::vector<int> shapeSegments = this->_shape->getLineSegments();

			for (auto& curve : this->_curves) {
				auto path = curve->generatePath();
				if (path.empty()) return;

				int vertsInShape = static_cast<int>(this->_shape->vertex.size());
				int edgeLoops = static_cast<int>(path.size());
				int segments = edgeLoops - 1;
				int vertCount = vertsInShape * edgeLoops;
				int triCount = static_cast<int>(shapeSegments.size()) * segments;
				int triIndexCount = triCount * 3;

				rawrbox::Mesh mesh;
				std::vector<uint16_t> triangleIndices(triIndexCount);
				std::vector<typename M::vertexBufferType> buff(vertCount);

				// Generate all of the vertices and normals
				int size = static_cast<int>(path.size());
				for (int i = 0; i < size; i++) {
					int offset = i * vertsInShape;
					for (int j = 0; j < vertsInShape; j++) {
						int id = offset + j;

						auto pos = rawrbox::Vector3f(this->_shape->vertex[j].x, this->_shape->vertex[j].y, 0.F);
						auto norm = rawrbox::Vector3f(this->_shape->normal[j].x, this->_shape->normal[j].y, 0.F);
						auto uv = rawrbox::Vector2f(this->_shape->u[j], path[i].vCoordinate);

						if constexpr (supportsNormals<typename M::vertexBufferType>) {
							buff[id] = rawrbox::VertexNormData(path[i].LocalToWorld(pos), uv, norm, {}, rawrbox::Colors::White());
						} else {
							buff[id] = rawrbox::VertexData(path[i].LocalToWorld(pos), uv, rawrbox::Colors::White());
						}
					}
				}

				int ti = 0;
				for (int i = 0; i < segments; i++) {
					int offset = i * vertsInShape;
					for (int l = 0; l < static_cast<int>(shapeSegments.size()); l += 2) {
						auto a = static_cast<uint16_t>(offset + shapeSegments[l]);
						auto b = static_cast<uint16_t>(offset + shapeSegments[l] + vertsInShape);
						auto c = static_cast<uint16_t>(offset + shapeSegments[l + 1] + vertsInShape);
						auto d = static_cast<uint16_t>(offset + shapeSegments[l + 1]);

						triangleIndices[ti++] = a; // 0
						triangleIndices[ti++] = b; // 1
						triangleIndices[ti++] = c; // 2

						triangleIndices[ti++] = d; // 3
						triangleIndices[ti++] = c; // 2
						triangleIndices[ti++] = a; // 0
					}
				}

				mesh.totalIndex = static_cast<uint16_t>(triIndexCount);
				mesh.totalVertex = static_cast<uint16_t>(vertCount);
				mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
				mesh.indices.insert(mesh.indices.end(), triangleIndices.begin(), triangleIndices.end());

				this->_mesh->merge(mesh);
			}
		}

		void draw() override {
			rawrbox::ModelBase<M>::draw();

			// Bind materials uniforms & textures ----
			rawrbox::MAIN_CAMERA->setModelTransform(this->getMatrix());

			this->_material->init();
			this->_material->bindTexture(*this->_mesh);
			this->_material->bindPipeline(*this->_mesh);
			this->_material->bindUniforms(*this->_mesh);
			this->_material->bindShaderResources();
			// -----------

			Diligent::DrawIndexedAttribs DrawAttrs;    // This is an indexed draw call
			DrawAttrs.IndexType = Diligent::VT_UINT16; // Index type
			DrawAttrs.NumIndices = this->_mesh->totalIndex;
			DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
			rawrbox::RENDERER->context()->DrawIndexed(DrawAttrs);
		}
	};
} // namespace rawrbox
