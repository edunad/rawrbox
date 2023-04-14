#pragma once

#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/color.hpp>

#include <bgfx/bgfx.h>

#include <memory>
#include <array>

namespace rawrBox {
	struct ModelVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		float u = 0;
		float v = 0;

		float nX = 0;
		float nY = 0;
		float nZ = 0;

		uint32_t abgr = 0xFFFFFFFF;

		ModelVertexData() = default;
		ModelVertexData(const rawrBox::Vector3f &pos, const rawrBox::Vector2f &uv, const rawrBox::Vector3f &normal, const rawrBox::Color cl = rawrBox::Colors::White) : x(pos.x), y(pos.y), z(pos.z), u(uv.x), v(uv.y), nX(normal.x), nY(normal.y), nZ(normal.z), abgr(rawrBox::Color::pack(cl)) {}
		ModelVertexData(float _x, float _y, float _z, float _u, float _v, float _nX, float _nY, float _nZ, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), nX(_nX), nY(_nY), nZ(_nZ), abgr(_abgr) {}
	};

	class ModelMeshData {
		public:
			virtual ~ModelMeshData() = default;

			uint16_t baseVertex;
			uint16_t baseIndex;
			uint16_t numVertices;
			uint16_t numIndices;

			std::vector<rawrBox::ModelVertexData> vertices;
			std::vector<uint16_t> indices;
	};

	class ModelMesh {
		private:
			std::string _name = "mesh";
			std::shared_ptr<rawrBox::ModelMeshData> _data = nullptr;

		public:
			std::array<float, 16> matrix;

			ModelMesh(const std::string &name_ = "mesh");
			virtual ~ModelMesh() = default;

			// UTILS ----
			const std::string &getName();

			std::shared_ptr<rawrBox::ModelMeshData>& getData();
			std::vector<rawrBox::ModelVertexData>& getVertices();
			std::vector<uint16_t>& getIndices();

			void generatePlane(const rawrBox::Vector3f& pos,const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal = {0, 0, 1}, const rawrBox::Color& cl = rawrBox::Colors::White);
			void generateCube(const rawrBox::Vector3f& pos,const rawrBox::Vector3f& size, const rawrBox::Vector3f& normal = {0, 0, 1}, const rawrBox::Color& cl = rawrBox::Colors::White);
			// ----
	};
}
