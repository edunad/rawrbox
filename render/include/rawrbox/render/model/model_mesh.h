#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/texture/base.h>

#include <bgfx/bgfx.h>
#include <bx/math.h>

#include <array>
#include <memory>

namespace rawrBox {
	struct ModelVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		uint32_t normal;
		float u;
		float v;

		uint32_t abgr = 0xFFFFFFFF;

		ModelVertexData() = default;
		ModelVertexData(const rawrBox::Vector3f& _pos, uint32_t _normal, float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : x(_pos.x), y(_pos.y), z(_pos.z), u(_u), v(_v), normal(_normal), abgr(rawrBox::Color::pack(cl)) {}
		ModelVertexData(float _x, float _y, float _z, uint32_t _normal, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), u(_u), v(_v), normal(_normal), abgr(_abgr) {}
	};

	struct ModelMeshData {
	public:
		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;

		std::shared_ptr<rawrBox::TextureBase> texture = nullptr;
		std::vector<rawrBox::ModelVertexData> vertices = {};
		std::vector<uint16_t> indices = {};

		std::array<float, 16> offsetMatrix;
		bool wireframe = false;

		virtual ~ModelMeshData() = default;
		ModelMeshData() {
			bx::mtxIdentity(offsetMatrix.data());
		};
	};

	class ModelMesh {
	private:
		std::string _name = "mesh";
		std::shared_ptr<rawrBox::ModelMeshData> _data = nullptr;

	public:
		ModelMesh(const std::string& name_ = "mesh");
		virtual ~ModelMesh() = default;

		// UTILS ----
		const std::string& getName();
		void setName(const std::string& name);

		std::shared_ptr<rawrBox::ModelMeshData>& getData();
		std::vector<rawrBox::ModelVertexData>& getVertices();
		std::vector<uint16_t>& getIndices();

		void setMatrix(const std::array<float, 16>& offset);
		void setTexture(const std::shared_ptr<rawrBox::TextureBase>& ptr);
		void setWireframe(bool wireframe);

		void generatePlane(const rawrBox::Vector3f& pos, const rawrBox::Vector2f& size, const rawrBox::Vector3f& normal = {0, 0, 1}, const rawrBox::Color& cl = rawrBox::Colors::White);
		void generateCube(const rawrBox::Vector3f& pos, const rawrBox::Vector3f& size, const rawrBox::Color& cl = rawrBox::Colors::White);
		void generateGrid(uint32_t size, const rawrBox::Vector3f& pos, const rawrBox::Color& cl = rawrBox::Colors::White);
		// ----
	};
} // namespace rawrBox
