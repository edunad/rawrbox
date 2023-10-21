
#include <rawrbox/render/models/spline.hpp>
#include <rawrbox/render/renderers/base.hpp>

namespace rawrbox {
	Spline::Spline(float subDivisions) : _subDivisions(subDivisions) {
		this->_mesh->setCulling(Diligent::CULL_MODE_NONE);
	}

	void Spline::addBezier(std::array<rawrbox::Vector3f, 4> points) {
		this->_curves.push_back(std::make_unique<rawrbox::BezierCurve>(points, this->_subDivisions));
	}

	void Spline::setExtrudeVerts(rawrbox::Mesh2DShape shape) {
		this->_shape = std::make_unique<rawrbox::Mesh2DShape>(shape);
	}

	void Spline::setTexture(rawrbox::TextureBase* texture) {
		this->_mesh->setTexture(texture);
	}

	void Spline::setColor(const rawrbox::Color& col) {
		this->_mesh->setColor(col);
	}

	rawrbox::Mesh& Spline::getMesh() {
		return *this->_mesh.get();
	}

	const std::vector<std::unique_ptr<rawrbox::BezierCurve>>& Spline::getCurves() {
		return this->_curves;
	}

	void Spline::addPoint(const rawrbox::Vector3f& p1, const rawrbox::Vector3f& p2, const rawrbox::Vector3f& p3, const rawrbox::Vector3f& p4) {
		this->addBezier({p1, p2, p3, p4});
	}

	void Spline::addPoint(const rawrbox::Vector4f& start, const rawrbox::Vector4f& end, float distance) {
		auto axisS = Vector3f::forward().rotateAroundOrigin(rawrbox::Vector3f::up(), rawrbox::MathUtils::toRad(start.w)) * distance;
		auto axisE = (-Vector3f::forward()).rotateAroundOrigin(rawrbox::Vector3f::up(), rawrbox::MathUtils::toRad(end.w)) * distance;

		this->addBezier({start.xyz(), start.xyz() + axisS, end.xyz() + axisE, end.xyz()});
	}

	void Spline::generateMesh() {
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
			std::vector<rawrbox::VertexNormBoneData> buff(vertCount);

			// Generate all of the vertices and normals
			int size = static_cast<int>(path.size());
			for (int i = 0; i < size; i++) {
				int offset = i * vertsInShape;
				for (int j = 0; j < vertsInShape; j++) {
					int id = offset + j;

					auto pos = rawrbox::Vector3f(this->_shape->vertex[j].x, this->_shape->vertex[j].y, 0.F);
					auto norm = rawrbox::Vector3f(this->_shape->normal[j].x, this->_shape->normal[j].y, 0.F);
					auto uv = rawrbox::Vector2f(this->_shape->u[j], path[i].vCoordinate);

					buff[id] = rawrbox::VertexNormBoneData(path[i].LocalToWorld(pos), uv, norm, {}, rawrbox::Colors::White());
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
	};

	void Spline::draw() {
		rawrbox::ModelBase::draw();

		// Bind materials uniforms & textures ----
		rawrbox::TRANSFORM = this->getMatrix();
		this->_material->bind(*this->_mesh);
		// -----------

		Diligent::DrawIndexedAttribs DrawAttrs;    // This is an indexed draw call
		DrawAttrs.IndexType = Diligent::VT_UINT16; // Index type
		DrawAttrs.NumIndices = this->_mesh->totalIndex;
		DrawAttrs.Flags = Diligent::DRAW_FLAG_VERIFY_ALL; // Verify the state of vertex and index buffers
		rawrbox::RENDERER->context->DrawIndexed(DrawAttrs);
	}
} // namespace rawrbox
