
#include <rawrbox/render/model/utils/mesh.hpp>

namespace rawrbox {
	rawrbox::Mesh MeshUtils::generateBBOX(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& bbox) {
		rawrbox::Mesh mesh = generateCube(pos, bbox.size(), rawrbox::Colorf(0.1F, 0.1F, 0.1F, 0.3F));

		mesh.setPos(pos);
		mesh.setCulling(BGFX_STATE_CULL_CCW);

		return mesh;
	}

	rawrbox::Mesh MeshUtils::generateLine(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Color& col) {
		rawrbox::Mesh mesh;
		mesh.setPos(a);

		std::array<rawrbox::VertexData, 2> buff = {
		    rawrbox::VertexData(a, {0, 0}, {}, col),
		    rawrbox::VertexData(b, {0, 0}, {}, col),
		};

		std::array<uint16_t, 2>
		    inds{0, 1};

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = 2;
		mesh.totalIndex = 2;

		mesh.lineMode = true;
		mesh.setOptimizable(false);

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		return mesh;
	}

	rawrbox::Mesh MeshUtils::generateTriangle(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		std::array<rawrbox::VertexData, 3> buff = {
		    rawrbox::VertexData(a, aUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, colA),
		    rawrbox::VertexData(b, bUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, colB),
		    rawrbox::VertexData(c, cUV, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, colC),
		};

		std::array<uint16_t, 3> inds{0, 1, 2};

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = 3;
		mesh.totalIndex = 3;

		// AABB ---
		auto calcAABB = [&mesh](const rawrbox::Vector3f& point) -> void {
			if (point.x < mesh.bbox.m_min.x) mesh.bbox.m_min.x = point.x;
			if (point.y < mesh.bbox.m_min.y) mesh.bbox.m_min.y = point.y;
			if (point.z < mesh.bbox.m_min.z) mesh.bbox.m_min.z = point.z;

			if (point.x > mesh.bbox.m_max.x) mesh.bbox.m_max.x = point.x;
			if (point.y > mesh.bbox.m_max.y) mesh.bbox.m_max.y = point.y;
			if (point.z > mesh.bbox.m_max.z) mesh.bbox.m_max.z = point.z;
		};

		calcAABB(a);
		calcAABB(b);
		calcAABB(c);

		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		return mesh;
	}

	rawrbox::Mesh MeshUtils::generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		rawrbox::Vector2f hSize = size / 2.F;
		std::array<rawrbox::VertexData, 4> buff = {
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, 0), {0, 1}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, 0), {1, 0}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, 0), {0, 0}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, 0), {1, 1}, {rawrbox::PackUtils::packNormal(1, 0, 0), 0}, cl),
		};

		std::array<uint16_t, 6> inds{
		    0, 1, 2,
		    0, 3, 1};

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;

		mesh.totalVertex = 4;
		mesh.totalIndex = 6;
		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		// AABB ---
		mesh.bbox.m_min = {-hSize.x, -hSize.y, 0};
		mesh.bbox.m_max = {hSize.x, hSize.y, 0};
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		return mesh;
	}

	rawrbox::Mesh MeshUtils::generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		rawrbox::Vector3f hSize = size / 2.F;

		auto nmrlB = rawrbox::PackUtils::packNormal(-1, 0, 0);
		auto nmrlF = rawrbox::PackUtils::packNormal(1, 0, 0);
		auto nmrlR = rawrbox::PackUtils::packNormal(0, 0, -1);
		auto nmrlL = rawrbox::PackUtils::packNormal(0, 0, 1);
		auto nmrlT = rawrbox::PackUtils::packNormal(0, -1, 0);
		auto nmrlBT = rawrbox::PackUtils::packNormal(0, 1, 0);

		std::array<rawrbox::VertexData, 24> buff = {
		    // Back
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlB, nmrlB}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlB, nmrlB}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlB, nmrlB}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlB, nmrlB}, cl),

		    // Front
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 0}, {nmrlF, nmrlF}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 1}, {nmrlF, nmrlF}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 0}, {nmrlF, nmrlF}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 1}, {nmrlF, nmrlF}, cl),

		    // Right
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlR, nmrlR}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {0, 1}, {nmrlR, nmrlR}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlR, nmrlR}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {0, 0}, {nmrlR, nmrlR}, cl),

		    // Left
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlL, nmrlL}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {1, 1}, {nmrlL, nmrlL}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlL, nmrlL}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {1, 0}, {nmrlL, nmrlL}, cl),

		    // Top
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, hSize.z), {1, 0}, {nmrlT, nmrlT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, -hSize.z), {0, 1}, {nmrlT, nmrlT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, hSize.y, hSize.z), {0, 0}, {nmrlT, nmrlT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, hSize.y, -hSize.z), {1, 1}, {nmrlT, nmrlT}, cl),

		    // Bottom
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, hSize.z), {1, 1}, {nmrlBT, nmrlBT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, -hSize.z), {0, 0}, {nmrlBT, nmrlBT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(-hSize.x, -hSize.y, hSize.z), {0, 1}, {nmrlBT, nmrlBT}, cl),
		    rawrbox::VertexData(rawrbox::Vector3f(hSize.x, -hSize.y, -hSize.z), {1, 0}, {nmrlBT, nmrlBT}, cl)};

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

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(inds.size());

		// AABB ---
		mesh.bbox.m_min = -hSize;
		mesh.bbox.m_max = hSize;
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		return mesh;
	}

	rawrbox::Mesh MeshUtils::generateAxis(float size, const rawrbox::Vector3f& pos) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		float hSize = size / 2.F;
		mesh.merge(generateCube(pos, {hSize * 2, 0.01F, 0.01F}, rawrbox::Colors::Red()));   // x
		mesh.merge(generateCube(pos, {0.01F, hSize * 2, 0.01F}, rawrbox::Colors::Green())); // y
		mesh.merge(generateCube(pos, {0.01F, 0.01F, hSize * 2}, rawrbox::Colors::Blue()));  // z

		// AABB ---
		mesh.bbox.m_min = {-hSize, -hSize, -hSize};
		mesh.bbox.m_max = {hSize, hSize, hSize};
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.setCulling(0);
		mesh.setTexture(rawrbox::WHITE_TEXTURE.get());

		return mesh;
	}

	// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L687
	// Does not support UV :( / normals
	rawrbox::Mesh MeshUtils::generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, uint32_t ratio, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		const float step = rawrbox::pi<float> * 2.0F / ratio;

		const uint32_t numVertices = ratio + 1;
		const uint32_t numIndices = ratio * 6;
		const uint32_t numLineListIndices = ratio * 4;

		std::vector<rawrbox::VertexData> buff = {};
		buff.resize(numVertices);

		std::vector<uint16_t> index = {};
		index.resize(numIndices + numLineListIndices);

		auto hSize = size / 2.F;
		hSize.y /= 2.F;

		buff[ratio] = rawrbox::VertexData(rawrbox::Vector3f(0, hSize.y, 0), {0, 0}, cl);

		for (uint32_t ii = 0; ii < ratio; ++ii) {
			const float angle = step * ii;

			const float angX = std::cos(angle) * hSize.x;
			const float angZ = std::sin(angle) * hSize.z;

			buff[ii] = rawrbox::VertexData(rawrbox::Vector3f(angZ, -hSize.y, angX), {0, 0}, cl);

			index[ii * 3 + 0] = uint16_t(ratio);
			index[ii * 3 + 1] = uint16_t((ii + 1) % ratio);
			index[ii * 3 + 2] = uint16_t(ii);

			index[ratio * 3 + ii * 3 + 0] = 0;
			index[ratio * 3 + ii * 3 + 1] = uint16_t(ii);
			index[ratio * 3 + ii * 3 + 2] = uint16_t((ii + 1) % ratio);

			index[numIndices + ii * 2 + 0] = uint16_t(ii);
			index[numIndices + ii * 2 + 1] = uint16_t(ratio);

			index[numIndices + ratio * 2 + ii * 2 + 0] = uint16_t(ii);
			index[numIndices + ratio * 2 + ii * 2 + 1] = uint16_t((ii + 1) % ratio);
		}

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(index.size());

		// AABB ---
		mesh.bbox.m_min = -hSize;
		mesh.bbox.m_max = hSize;
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), index.begin(), index.end());

		return mesh;
	}

	// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L750
	// Does not support UV :( / normals
	rawrbox::Mesh MeshUtils::generateCylinder(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, uint32_t ratio, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		const float step = rawrbox::pi<float> * 2.0F / ratio;
		const uint32_t numVertices = ratio * 2;
		const uint32_t numIndices = ratio * 12;
		const uint32_t numLineListIndices = ratio * 6;
		const rawrbox::Vector3f hSize = size / 2.F;

		std::vector<rawrbox::VertexData> buff = {};
		buff.resize(numVertices);

		std::vector<uint16_t> index = {};
		index.resize(numIndices + numLineListIndices);

		for (uint32_t ii = 0; ii < ratio; ++ii) {
			const float angle = step * ii;

			const float angX = std::cos(angle) * hSize.x;
			const float angZ = std::sin(angle) * hSize.z;

			buff[ii] = rawrbox::VertexData(rawrbox::Vector3f(angX, hSize.y, angZ), {0, 0}, cl);
			buff[ii + ratio] = rawrbox::VertexData(rawrbox::Vector3f(angX, -hSize.y, angZ), {0, 0}, cl);

			index[ii * 6 + 0] = uint16_t(ii + ratio);
			index[ii * 6 + 1] = uint16_t((ii + 1) % ratio);
			index[ii * 6 + 2] = uint16_t(ii);
			index[ii * 6 + 3] = uint16_t(ii + ratio);
			index[ii * 6 + 4] = uint16_t((ii + 1) % ratio + ratio);
			index[ii * 6 + 5] = uint16_t((ii + 1) % ratio);

			index[ratio * 6 + ii * 6 + 0] = uint16_t(0);
			index[ratio * 6 + ii * 6 + 1] = uint16_t(ii);
			index[ratio * 6 + ii * 6 + 2] = uint16_t((ii + 1) % ratio);
			index[ratio * 6 + ii * 6 + 3] = uint16_t(ratio);
			index[ratio * 6 + ii * 6 + 4] = uint16_t((ii + 1) % ratio + ratio);
			index[ratio * 6 + ii * 6 + 5] = uint16_t(ii + ratio);

			index[numIndices + ii * 2 + 0] = uint16_t(ii);
			index[numIndices + ii * 2 + 1] = uint16_t(ii + ratio);

			index[numIndices + ratio * 2 + ii * 2 + 0] = uint16_t(ii);
			index[numIndices + ratio * 2 + ii * 2 + 1] = uint16_t((ii + 1) % ratio);

			index[numIndices + ratio * 4 + ii * 2 + 0] = uint16_t(ratio + ii);
			index[numIndices + ratio * 4 + ii * 2 + 1] = uint16_t(ratio + (ii + 1) % ratio);
		}

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(index.size());

		// AABB ---
		mesh.bbox.m_min = -hSize;
		mesh.bbox.m_max = hSize;
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), index.begin(), index.end());

		return mesh;
	}

	// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L640
	// Does not support UV :( / normals
	rawrbox::Mesh MeshUtils::generateSphere(const rawrbox::Vector3f& pos, float size, uint32_t ratio, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

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

		std::vector<rawrbox::VertexData> buff = {};

		std::function<void(const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint32_t ratio)> triangle;
		triangle = [&triangle, &buff, &cl](const rawrbox::Vector3f& _v0, const rawrbox::Vector3f& _v1, const rawrbox::Vector3f& _v2, float _scale, uint32_t ratio) {
			if (0 == ratio) {
				buff.push_back(rawrbox::VertexData(_v0, {1, 1}, cl));
				buff.push_back(rawrbox::VertexData(_v2, {1, 0}, cl));
				buff.push_back(rawrbox::VertexData(_v1, {0, 1}, cl));
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

		uint32_t numLineListIndices = bgfx::topologyConvert(
		    bgfx::TopologyConvert::TriListToLineList, nullptr, 0, trilist.data(), numIndices, false);

		std::vector<uint16_t> inds = {};
		inds.resize(numLineListIndices * sizeof(uint16_t));

		bgfx::topologyConvert(
		    bgfx::TopologyConvert::TriListToLineList, inds.data(), numLineListIndices * sizeof(uint16_t), trilist.data(), numIndices, false);

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(inds.size());

		// AABB ---
		auto scale = rawrbox::Vector3f{hSize, hSize, hSize};

		mesh.bbox.m_min = -scale;
		mesh.bbox.m_max = scale;
		mesh.bbox.m_size = mesh.bbox.m_min.abs() + mesh.bbox.m_max.abs();
		// -----

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());
		return mesh;
	}

	rawrbox::Mesh MeshUtils::generateMesh(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, uint32_t subDivs, const rawrbox::Colorf& cl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		std::vector<rawrbox::VertexData> buff = {};
		auto uvScale = 1.0F / static_cast<float>(subDivs - 1);

		for (int y = 0; y < subDivs; y++) {
			for (int x = 0; x < subDivs; x++) {
				auto xF = static_cast<float>(x);
				auto yF = static_cast<float>(y);

				rawrbox::Vector2f posDiv = {xF, yF};
				posDiv /= static_cast<float>(subDivs - 1);
				posDiv *= size;

				posDiv -= size / 2;

				buff.push_back(rawrbox::VertexData(
				    pos + Vector3f{posDiv.x, 0, posDiv.y},
				    {uvScale * xF,
					uvScale * yF},
				    {rawrbox::PackUtils::packNormal(0, 1, 0), 0},
				    cl));
			}
		}

		std::vector<uint16_t> inds = {};
		auto subDivsUI16 = static_cast<uint16_t>(subDivs);
		inds.reserve(buff.size() / 4 * 6);
		for (size_t y = 0; y < subDivs - 1; y++) {
			auto yOffset = static_cast<uint16_t>(y * subDivsUI16);

			for (size_t x = 0; x < subDivs - 1; x++) {
				uint16_t index = yOffset + static_cast<uint16_t>(x);

				inds.push_back(index + 1);
				inds.push_back(index + subDivsUI16);
				inds.push_back(index);

				inds.push_back(index + subDivsUI16 + 1);
				inds.push_back(index + subDivsUI16);
				inds.push_back(index + 1);
			}
		}

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(inds.size());

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		return mesh;
	}

	// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
	rawrbox::Mesh MeshUtils::generateGrid(uint16_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl, const rawrbox::Colorf& borderCl) {
		rawrbox::Mesh mesh;
		mesh.setPos(pos);

		std::vector<rawrbox::VertexData> buff = {};
		std::vector<uint16_t> inds = {};

		float step = 1.F;
		for (uint16_t j = 0; j <= size; ++j) {
			for (uint16_t i = 0; i <= size; ++i) {
				float x = static_cast<float>(i) / static_cast<float>(step);
				float y = 0;
				float z = static_cast<float>(j) / static_cast<float>(step);
				auto col = cl;

				if (j == 0 || i == 0 || j >= size || i >= size) col = borderCl;
				buff.emplace_back(rawrbox::VertexData{rawrbox::Vector3f(pos.x - static_cast<float>(size / 2), pos.y, pos.z - static_cast<float>(size / 2)) + rawrbox::Vector3f(x, y, z), {0, 0}, col});
			}
		}

		for (uint16_t j = 0; j < size; ++j) {
			for (uint16_t i = 0; i < size; ++i) {

				uint16_t row1 = j * (size + 1);
				uint16_t row2 = (j + 1) * (size + 1);

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

		mesh.baseVertex = 0;
		mesh.baseIndex = 0;
		mesh.totalVertex = static_cast<uint16_t>(buff.size());
		mesh.totalIndex = static_cast<uint16_t>(inds.size());

		mesh.vertices.insert(mesh.vertices.end(), buff.begin(), buff.end());
		mesh.indices.insert(mesh.indices.end(), inds.begin(), inds.end());

		mesh.lineMode = true;
		return mesh;
	}
} // namespace rawrbox
