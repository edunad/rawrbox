#pragma once

#include <rawrbox/render/materials/unlit.hpp>
#include <rawrbox/render/utils/topology.hpp>

namespace rawrbox {
	class MeshUtils {
	public:
		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateBBOX(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& bbox) {
			auto mesh = generateCube<M>(pos, bbox.size(), rawrbox::Colorf(1.0F, 0.1F, 0.1F, 1.0F));
			mesh.setWireframe(true);

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateLine(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Color& col) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			mesh.lineMode = true;
			mesh.setOptimizable(false);

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				mesh.vertices = {
				    rawrbox::VertexNormData(a, rawrbox::Vector2f(), {}, {}, col),
				    rawrbox::VertexNormData(b, rawrbox::Vector2f(), {}, {}, col),
				};
			} else {
				mesh.vertices = {
				    rawrbox::VertexData(a, rawrbox::Vector2f(), col),
				    rawrbox::VertexData(b, rawrbox::Vector2f(), col),
				};
			}

			mesh.indices = {0, 1};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = 2;
			mesh.totalIndex = 2;

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateTriangle(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};
			rawrbox::Vector3f norm = -rawrbox::Vector3f::forward();

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				mesh.vertices = {
				    rawrbox::VertexNormData(pos + a, aUV, norm, norm, colA),
				    rawrbox::VertexNormData(pos + b, bUV, norm, norm, colB),
				    rawrbox::VertexNormData(pos + c, cUV, norm, norm, colC),
				};
			} else {
				mesh.vertices = {
				    rawrbox::VertexData(pos + a, aUV, colA),
				    rawrbox::VertexData(pos + b, bUV, colB),
				    rawrbox::VertexData(pos + c, cUV, colC),
				};
			};

			mesh.indices = {0, 1, 2};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = 3;
			mesh.totalIndex = 3;

			// AABB ---
			auto calcAABB = [&mesh](const rawrbox::Vector3f& point) -> void {
				if (point.x < mesh.bbox._min.x) mesh.bbox._min.x = point.x;
				if (point.y < mesh.bbox._min.y) mesh.bbox._min.y = point.y;
				if (point.z < mesh.bbox._min.z) mesh.bbox._min.z = point.z;

				if (point.x > mesh.bbox._max.x) mesh.bbox._max.x = point.x;
				if (point.y > mesh.bbox._max.y) mesh.bbox._max.y = point.y;
				if (point.z > mesh.bbox._max.z) mesh.bbox._max.z = point.z;
			};

			calcAABB(a);
			calcAABB(b);
			calcAABB(c);

			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};
			rawrbox::Vector2f hSize = size / 2.F;
			rawrbox::Vector3f norm = -rawrbox::Vector3f::forward();

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				mesh.vertices = {
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, 0), rawrbox::Vector2f(0, 1), norm, norm, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, 0), rawrbox::Vector2f(1, 0), norm, norm, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, 0), rawrbox::Vector2f(0, 0), norm, norm, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, 0), rawrbox::Vector2f(1, 1), norm, norm, cl),
				};
			} else {
				mesh.vertices = {
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, 0), rawrbox::Vector2f(0, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, 0), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, 0), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, 0), rawrbox::Vector2f(1, 1), cl),
				};
			}

			mesh.indices = {
			    0, 1, 2,
			    0, 3, 1};

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;

			mesh.totalVertex = 4;
			mesh.totalIndex = 6;

			// AABB ---
			mesh.bbox._min = {-hSize.x, -hSize.y, 0};
			mesh.bbox._max = {hSize.x, hSize.y, 0};
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};
			rawrbox::Vector3f hSize = size / 2.F;

			auto nmrlB = rawrbox::Vector3f(0, 0, 1);
			auto nmrlF = rawrbox::Vector3f(0, 0, -1);
			auto nmrlR = rawrbox::Vector3f(1, 0, 0);
			auto nmrlL = rawrbox::Vector3f(-1, 0, 0);
			auto nmrlT = rawrbox::Vector3f(0, 1, 0);
			auto nmrlBT = rawrbox::Vector3f(0, -1, 0);

			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				mesh.vertices = {
				    // Back
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), nmrlB, nmrlB, cl),   // A
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), nmrlB, nmrlB, cl), // B
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), nmrlB, nmrlB, cl),  // C
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), nmrlB, nmrlB, cl),  // D

				    // Front
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), nmrlF, nmrlF, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), nmrlF, nmrlF, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), nmrlF, nmrlF, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), nmrlF, nmrlF, cl),

				    // Right
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), nmrlR, nmrlR, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), nmrlR, nmrlR, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), nmrlR, nmrlR, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), nmrlR, nmrlR, cl),

				    // Left
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), nmrlL, nmrlL, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), nmrlL, nmrlL, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), nmrlL, nmrlL, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), nmrlL, nmrlL, cl),

				    // Top
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), nmrlT, nmrlT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), nmrlT, nmrlT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), nmrlT, nmrlT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), nmrlT, nmrlT, cl),

				    // Bottom
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), nmrlBT, nmrlBT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), nmrlBT, nmrlBT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), nmrlBT, nmrlBT, cl),
				    rawrbox::VertexNormData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), nmrlBT, nmrlBT, cl)};
			} else {
				mesh.vertices = {
				    // Back
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), cl),   // A
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), cl), // B
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), cl),  // C
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), cl),  // D

				    // Front
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), cl),

				    // Right
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), cl),

				    // Left
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), cl),

				    // Top
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(1, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), rawrbox::Vector2f(0, 1), cl),

				    // Bottom
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(0, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(1, 1), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), rawrbox::Vector2f(1, 0), cl),
				    rawrbox::VertexData(pos + rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), rawrbox::Vector2f(0, 1), cl)};
			}

			std::vector<uint16_t> inds = {};
			constexpr std::array<uint16_t, 6> cubeInd = {
			    0, 1, 2,
			    0, 3, 1};

			for (uint16_t v = 0; v < 24; v += 4) {
				for (const auto& ind : cubeInd) {
					inds.push_back(ind + v);
				}
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(mesh.vertices.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			mesh.bbox._min = -hSize;
			mesh.bbox._max = hSize;
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateAxis(float size, const rawrbox::Vector3f& pos) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			float hSize = size / 2.F;
			mesh.merge(generateCube<M>(pos, {hSize * 2, 0.01F, 0.01F}, rawrbox::Colors::Red())); // x

			auto xCone = generateCone<M>(pos + rawrbox::Vector3f(0, hSize, 0), {0.08F, 0.1F, 0.08F}, 6, rawrbox::Colors::Red());
			xCone.rotateVertices(rawrbox::MathUtils::toRad(90.F), {0, 0, 1});
			mesh.merge(xCone);

			mesh.merge(generateCube<M>(pos, {0.01F, hSize * 2, 0.01F}, rawrbox::Colors::Green()));                                // y
			mesh.merge(generateCone<M>(pos + rawrbox::Vector3f(0, hSize, 0), {0.08F, 0.1F, 0.08F}, 6, rawrbox::Colors::Green())); // y

			auto zCone = generateCone<M>(pos + rawrbox::Vector3f(0, hSize, 0), {0.08F, 0.1F, 0.08F}, 6, rawrbox::Colors::Blue());
			zCone.rotateVertices(rawrbox::MathUtils::toRad(-90.F), {1, 0, 0});
			mesh.merge(zCone);

			mesh.merge(generateCube<M>(pos, {0.01F, 0.01F, hSize * 2}, rawrbox::Colors::Blue())); // z

			// AABB ---
			mesh.bbox._min = {-hSize, -hSize, -hSize};
			mesh.bbox._max = {hSize, hSize, hSize};
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateArrow(float size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			float hSize = 0.5F;
			auto offset = rawrbox::Vector3f(0, hSize / 2.F, 0) * size;

			mesh.merge(generateCone<M>(pos + offset, rawrbox::Vector3f(hSize, 1.F, hSize) * size, 6, cl));
			mesh.merge(generateCube<M>(pos - offset, rawrbox::Vector3f(0.15F, hSize, 0.15F) * size, cl));

			// AABB ---
			mesh.bbox._min = rawrbox::Vector3f(-hSize, -hSize, -hSize) * size;
			mesh.bbox._max = rawrbox::Vector3f(hSize, hSize, hSize) * size;
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L687
		// Does not support UV :( / normals
		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const uint16_t ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			if (ratio % 3 != 0) throw rawrbox::Logger::err("RawrBox-MeshUtils", "'generateCone' ratio '{}' needs to be divisible by 3", ratio);
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				throw rawrbox::Logger::err("RawrBox-MeshUtils", "'generateCone' does not support normals");
			}

			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};
			const float step = rawrbox::pi<float> * 2.0F / ratio;

			const uint16_t numVertices = ratio + 1;
			const uint16_t numIndices = ratio * 6;
			const uint16_t numLineListIndices = ratio * 4;

			mesh.vertices.resize(numVertices);
			mesh.indices.resize(numIndices + numLineListIndices);

			auto hSize = size / 2.F;
			hSize.y /= 2.F;

			mesh.vertices[ratio] = rawrbox::VertexData(pos + rawrbox::Vector3f(0, hSize.y, 0), rawrbox::Vector2f(0, 0), cl);

			for (uint16_t ii = 0; ii < ratio; ++ii) {
				const float angle = step * ii;

				const float angX = std::cos(angle) * hSize.x;
				const float angZ = std::sin(angle) * hSize.z;

				mesh.vertices[ii] = rawrbox::VertexData(pos + rawrbox::Vector3f(angZ, -hSize.y, angX), rawrbox::Vector2f(0, 0), cl);

				mesh.indices[ii * 3 + 0] = ratio;
				mesh.indices[ii * 3 + 1] = (ii + 1) % ratio;
				mesh.indices[ii * 3 + 2] = ii;

				mesh.indices[ratio * 3 + ii * 3 + 0] = 0;
				mesh.indices[ratio * 3 + ii * 3 + 1] = ii;
				mesh.indices[ratio * 3 + ii * 3 + 2] = (ii + 1) % ratio;

				mesh.indices[numIndices + ii * 2 + 0] = ii;
				mesh.indices[numIndices + ii * 2 + 1] = ratio;

				mesh.indices[numIndices + ratio * 2 + ii * 2 + 0] = ii;
				mesh.indices[numIndices + ratio * 2 + ii * 2 + 1] = (ii + 1) % ratio;
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = numVertices;
			mesh.totalIndex = numIndices + numLineListIndices;

			// AABB ---
			mesh.bbox._min = -hSize;
			mesh.bbox._max = hSize;
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L750
		// Does not support UV :( / normals
		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateCylinder(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const uint16_t ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				throw rawrbox::Logger::err("RawrBox-MeshUtils", "'generateCylinder' does not support normals");
			}

			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			const float step = rawrbox::pi<float> * 2.0F / ratio;
			const uint16_t numVertices = ratio * 2;
			const uint16_t numIndices = ratio * 12;
			const uint16_t numLineListIndices = ratio * 6;
			const rawrbox::Vector3f hSize = size / 2.F;

			mesh.vertices.resize(numVertices);
			mesh.indices.resize(numIndices + numLineListIndices);

			for (uint16_t ii = 0; ii < ratio; ++ii) {
				const float angle = step * ii;

				const float angX = std::cos(angle) * hSize.x;
				const float angZ = std::sin(angle) * hSize.z;

				mesh.vertices[ii] = rawrbox::VertexData(pos + rawrbox::Vector3f(angX, hSize.y, angZ), rawrbox::Vector2f(0, 0), cl);
				mesh.vertices[ii + ratio] = rawrbox::VertexData(pos + rawrbox::Vector3f(angX, -hSize.y, angZ), rawrbox::Vector2f(0, 0), cl);

				mesh.indices[ii * 6 + 0] = ii + ratio;
				mesh.indices[ii * 6 + 1] = (ii + 1) % ratio;
				mesh.indices[ii * 6 + 2] = ii;
				mesh.indices[ii * 6 + 3] = ii + ratio;
				mesh.indices[ii * 6 + 4] = (ii + 1) % ratio + ratio;
				mesh.indices[ii * 6 + 5] = (ii + 1) % ratio;

				mesh.indices[ratio * 6 + ii * 6 + 0] = 0;
				mesh.indices[ratio * 6 + ii * 6 + 1] = ii;
				mesh.indices[ratio * 6 + ii * 6 + 2] = (ii + 1) % ratio;
				mesh.indices[ratio * 6 + ii * 6 + 3] = ratio;
				mesh.indices[ratio * 6 + ii * 6 + 4] = (ii + 1) % ratio + ratio;
				mesh.indices[ratio * 6 + ii * 6 + 5] = ii + ratio;

				mesh.indices[numIndices + ii * 2 + 0] = ii;
				mesh.indices[numIndices + ii * 2 + 1] = ii + ratio;

				mesh.indices[numIndices + ratio * 2 + ii * 2 + 0] = ii;
				mesh.indices[numIndices + ratio * 2 + ii * 2 + 1] = (ii + 1) % ratio;

				mesh.indices[numIndices + ratio * 4 + ii * 2 + 0] = ratio + ii;
				mesh.indices[numIndices + ratio * 4 + ii * 2 + 1] = ratio + (ii + 1) % ratio;
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = numVertices;
			mesh.totalIndex = numIndices + numLineListIndices;

			// AABB ---
			mesh.bbox._min = -hSize;
			mesh.bbox._max = hSize;
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			return mesh;
		}

		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L640
		// Does not support UV :( / normals
		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateSphere(const rawrbox::Vector3f& pos, float size, uint32_t ratio = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				throw rawrbox::Logger::err("RawrBox-MeshUtils", "'generateSphere' does not support normals");
			}

			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			const float golden = 1.6180339887F;
			const float len = std::sqrt(golden * golden + 1.0F);
			const float hSize = size / 2.F;
			const float ss = 1.0F / len * hSize;
			const float ll = ss * golden;

			const std::array<rawrbox::Vector3f, 32> vv = {
			    rawrbox::Vector3f{-ll, 0.0F, -ss},
			    rawrbox::Vector3f{ll, 0.0F, -ss},
			    rawrbox::Vector3f{ll, 0.0F, ss},
			    rawrbox::Vector3f{-ll, 0.0F, ss},

			    rawrbox::Vector3f{-ss, ll, 0.0F},
			    rawrbox::Vector3f{ss, ll, 0.0F},
			    rawrbox::Vector3f{ss, -ll, 0.0F},
			    rawrbox::Vector3f{-ss, -ll, 0.0F},

			    rawrbox::Vector3f{0.0F, -ss, ll},
			    rawrbox::Vector3f{0.0F, ss, ll},
			    rawrbox::Vector3f{0.0F, ss, -ll},
			    rawrbox::Vector3f{0.0F, -ss, -ll},
			};

			std::vector<typename M::vertexBufferType> buff = {};

			std::function<void(const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint32_t ratio)> triangle;
			triangle = [&pos, &triangle, &buff, &cl](const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint32_t ratio) {
				if (0 == ratio) {
					buff.push_back(rawrbox::VertexData(pos + _v0, rawrbox::Vector2f(1, 1), cl));
					buff.push_back(rawrbox::VertexData(pos + _v2, rawrbox::Vector2f(1, 0), cl));
					buff.push_back(rawrbox::VertexData(pos + _v1, rawrbox::Vector2f(0, 1), cl));
				} else {
					const rawrbox::Vector3f v01 = (_v0 + _v1).normalized() * _scale;
					const rawrbox::Vector3f v12 = (_v1 + _v2).normalized() * _scale;
					const rawrbox::Vector3f v20 = (_v2 + _v0).normalized() * _scale;

					--ratio;
					triangle(_v0, v01, v20, _scale, ratio);
					triangle(_v1, v12, v01, _scale, ratio);
					triangle(_v2, v20, v12, _scale, ratio);
					triangle(v01, v12, v20, _scale, ratio);
				}
			};

			triangle(vv[0], vv[4], vv[3], hSize, ratio);
			triangle(vv[0], vv[10], vv[4], hSize, ratio);
			triangle(vv[4], vv[10], vv[5], hSize, ratio);
			triangle(vv[5], vv[10], vv[1], hSize, ratio);
			triangle(vv[5], vv[1], vv[2], hSize, ratio);
			triangle(vv[5], vv[2], vv[9], hSize, ratio);
			triangle(vv[5], vv[9], vv[4], hSize, ratio);
			triangle(vv[3], vv[4], vv[9], hSize, ratio);

			triangle(vv[0], vv[3], vv[7], hSize, ratio);
			triangle(vv[0], vv[7], vv[11], hSize, ratio);
			triangle(vv[11], vv[7], vv[6], hSize, ratio);
			triangle(vv[11], vv[6], vv[1], hSize, ratio);
			triangle(vv[1], vv[6], vv[2], hSize, ratio);
			triangle(vv[2], vv[6], vv[8], hSize, ratio);
			triangle(vv[8], vv[6], vv[7], hSize, ratio);
			triangle(vv[8], vv[7], vv[3], hSize, ratio);

			triangle(vv[0], vv[11], vv[10], hSize, ratio);
			triangle(vv[1], vv[10], vv[11], hSize, ratio);
			triangle(vv[2], vv[8], vv[9], hSize, ratio);
			triangle(vv[3], vv[9], vv[8], hSize, ratio);

			// ----------

			auto numIndices = static_cast<uint32_t>(buff.size());

			std::vector<uint16_t> trilist = {};
			trilist.resize(numIndices);
			for (uint32_t ii = 0; ii < numIndices; ++ii) {
				trilist[ii] = uint16_t(ii);
			}

			uint32_t numLineListIndices = rawrbox::TopologyUtils::triToLine(nullptr, 0, trilist.data(), numIndices, false);

			std::vector<uint16_t> inds = {};
			inds.resize(numLineListIndices * sizeof(uint16_t));

			rawrbox::TopologyUtils::triToLine(inds.data(), numLineListIndices * sizeof(uint16_t), trilist.data(), numIndices, false);

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = static_cast<uint16_t>(buff.size());
			mesh.totalIndex = static_cast<uint16_t>(inds.size());

			// AABB ---
			auto scale = rawrbox::Vector3f{hSize, hSize, hSize};

			mesh.bbox._min = -scale;
			mesh.bbox._max = scale;
			mesh.bbox._size = mesh.bbox._min.abs() + mesh.bbox._max.abs();
			// -----

			mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
			mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());
			return mesh;
		}

		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateMesh(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, uint16_t subDivs = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White()) {
			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			auto uvScale = 1.0F / static_cast<float>(subDivs - 1);
			const uint16_t vertSize = subDivs * subDivs;
			mesh.vertices.reserve(vertSize);

			for (uint32_t y = 0; y < subDivs; y++) {
				for (uint32_t x = 0; x < subDivs; x++) {
					auto xF = static_cast<float>(x);
					auto yF = static_cast<float>(y);

					rawrbox::Vector2f posDiv = {xF, yF};
					posDiv /= static_cast<float>(subDivs - 1);
					posDiv *= size;
					posDiv -= size / 2;

					if constexpr (supportsNormals<typename M::vertexBufferType>) {
						mesh.vertices.push_back(rawrbox::VertexNormData(
						    Vector3f(posDiv.x + pos.x, pos.y, posDiv.y + pos.z),
						    rawrbox::Vector2(uvScale * xF,
							uvScale * yF),
						    rawrbox::Vector3f(0, 1, 0), {},
						    cl));
					} else {
						mesh.vertices.push_back(rawrbox::VertexData(
						    pos + Vector3f(posDiv.x + pos.x, pos.y, posDiv.y + pos.z),
						    rawrbox::Vector2(uvScale * xF,
							uvScale * yF),
						    cl));
					}
				}
			}

			const uint16_t indcSize = ((subDivs - 1) * (subDivs - 1)) * 6;
			mesh.indices.reserve(indcSize);

			for (size_t y = 0; y < subDivs - 1; y++) {
				auto yOffset = static_cast<uint16_t>(y * subDivs);

				for (size_t x = 0; x < subDivs - 1; x++) {
					uint16_t index = yOffset + static_cast<uint16_t>(x);

					mesh.indices.push_back(index + 1);
					mesh.indices.push_back(index + subDivs);
					mesh.indices.push_back(index);

					mesh.indices.push_back(index + subDivs + 1);
					mesh.indices.push_back(index + subDivs);
					mesh.indices.push_back(index + 1);
				}
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = vertSize;
			mesh.totalIndex = indcSize;

			return mesh;
		}

		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		template <typename M = rawrbox::MaterialUnlit>
		static rawrbox::Mesh<typename M::vertexBufferType> generateGrid(uint16_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::Gray().strength(0.4F), const rawrbox::Colorf& borderCl = rawrbox::Colors::Transparent()) {
			if constexpr (supportsNormals<typename M::vertexBufferType>) {
				throw rawrbox::Logger::err("RawrBox-MeshUtils", "'generateGrid' does not support normals");
			}

			rawrbox::Mesh<typename M::vertexBufferType> mesh = {};

			const uint16_t vertSize = size * size;
			mesh.vertices.reserve(vertSize);

			float step = 1.F;
			for (uint16_t j = 0; j <= size; ++j) {
				for (uint16_t i = 0; i <= size; ++i) {
					float x = static_cast<float>(i) / static_cast<float>(step);
					float y = 0;
					float z = static_cast<float>(j) / static_cast<float>(step);
					auto col = cl;

					if (j == 0 || i == 0 || j >= size || i >= size) col = borderCl;
					mesh.vertices.push_back(rawrbox::VertexData(pos + rawrbox::Vector3f(pos.x - static_cast<float>(size / 2), pos.y, pos.z - static_cast<float>(size / 2)) + rawrbox::Vector3f(x, y, z), rawrbox::Vector2f(0, 0), col));
				}
			}

			const uint16_t indcSize = vertSize * 8;
			mesh.indices.reserve(indcSize);

			for (uint16_t j = 0; j < size; ++j) {
				for (uint16_t i = 0; i < size; ++i) {

					uint16_t row1 = j * (size + 1);
					uint16_t row2 = (j + 1) * (size + 1);

					mesh.indices.push_back(row1 + i);
					mesh.indices.push_back(row1 + i + 1);
					mesh.indices.push_back(row1 + i + 1);
					mesh.indices.push_back(row2 + i + 1);

					mesh.indices.push_back(row2 + i + 1);
					mesh.indices.push_back(row2 + i);
					mesh.indices.push_back(row2 + i);
					mesh.indices.push_back(row1 + i);
				}
			}

			mesh.baseVertex = 0;
			mesh.baseIndex = 0;
			mesh.totalVertex = vertSize;
			mesh.totalIndex = indcSize;
			mesh.lineMode = true;

			return mesh;
		}
	};
} // namespace rawrbox
