#pragma once

#include <rawrbox/render/materials/base.hpp>
#include <rawrbox/utils/pack.hpp>

#include <functional>

namespace rawrbox {
	class MeshUtils {
	public:
		static Mesh generateBBOX(const Vector3f& pos, const BBOXf& bbox);
		static Mesh generateLine(const Vector3f& a, const Vector3f& b, const Color& col);
		static Mesh generateTriangle(const Vector3f& pos, const Vector3f& a, const Vector2f& aUV, const Color& colA, const Vector3f& b, const Vector2f& bUV, const Color& colB, const Vector3f& c, const Vector2f& cUV, const Color& colC);
		static Mesh generatePlane(const Vector3f& pos, const Vector2f& size, const Colorf& cl = Colors::White());
		static Mesh generateCube(const Vector3f& pos, const Vector3f& size, const Colorf& cl = Colors::White());
		static Mesh generateAxis(float size, const Vector3f& pos);
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L687
		// Does not support UV :( / normals
		static Mesh generateCone(const Vector3f& pos, const Vector3f& size, uint32_t ratio = 12, const Colorf& cl = Colors::White());
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L750
		// Does not support UV :( / normals
		static Mesh generateCylinder(const Vector3f& pos, const Vector3f& size, uint32_t ratio = 12, const Colorf& cl = Colors::White());
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L640
		// Does not support UV :( / normals
		static Mesh generateSphere(const Vector3f& pos, float size, uint32_t ratio = 1, const Colorf& cl = Colors::White());
		static Mesh generateMesh(const Vector3f& pos, const Vector2f& size, uint32_t subDivs = 1, const Colorf& cl = Colors::White());
		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		static Mesh generateGrid(uint16_t size, const Vector3f& pos, const Colorf& cl = Colors::DarkGray(), const Colorf& borderCl = Colors::Transparent());
	};
} // namespace rawrbox
