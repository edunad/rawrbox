#pragma once

#include <rawrbox/render_temp/materials/base.hpp>

namespace rawrbox {
	class MeshUtils {
	public:
		static rawrbox::Mesh generateBBOX(const rawrbox::Vector3f& pos, const rawrbox::BBOXf& bbox);
		static rawrbox::Mesh generateLine(const rawrbox::Vector3f& a, const rawrbox::Vector3f& b, const rawrbox::Color& col);
		static rawrbox::Mesh generateTriangle(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& a, const rawrbox::Vector2f& aUV, const rawrbox::Color& colA, const rawrbox::Vector3f& b, const rawrbox::Vector2f& bUV, const rawrbox::Color& colB, const rawrbox::Vector3f& c, const rawrbox::Vector2f& cUV, const rawrbox::Color& colC);
		static rawrbox::Mesh generatePlane(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		static rawrbox::Mesh generateCube(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		static rawrbox::Mesh generateAxis(float size, const rawrbox::Vector3f& pos);
		static rawrbox::Mesh generateArrow(float size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L687
		// Does not support UV :( / normals
		static rawrbox::Mesh generateCone(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, uint32_t ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L750
		// Does not support UV :( / normals
		static rawrbox::Mesh generateCylinder(const rawrbox::Vector3f& pos, const rawrbox::Vector3f& size, uint32_t ratio = 12, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/common/debugdraw/debugdraw.cpp#L640
		// Does not support UV :( / normals
		static rawrbox::Mesh generateSphere(const rawrbox::Vector3f& pos, float size, uint32_t ratio = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		static rawrbox::Mesh generateMesh(const rawrbox::Vector3f& pos, const rawrbox::Vector2f& size, uint32_t subDivs = 1, const rawrbox::Colorf& cl = rawrbox::Colors::White());
		// Adapted from : https://stackoverflow.com/questions/58494179/how-to-create-a-grid-in-opengl-and-drawing-it-with-lines
		static rawrbox::Mesh generateGrid(uint16_t size, const rawrbox::Vector3f& pos, const rawrbox::Colorf& cl = rawrbox::Colors::Gray().strength(0.4F), const rawrbox::Colorf& borderCl = rawrbox::Colors::Transparent());
	};
} // namespace rawrbox
