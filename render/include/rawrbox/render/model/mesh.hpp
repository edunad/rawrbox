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

	struct MeshVertexData {
		float x = 0;
		float y = 0;
		float z = 0;

		uint32_t normal = 0;
		uint32_t tangent = 0;

		float u = 0;
		float v = 0;

		uint32_t abgr = 0xFFFFFFFF;

		MeshVertexData() = default;
		MeshVertexData(const rawrBox::Vector3f& _pos, uint32_t _normal, uint32_t _tangent, float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : x(_pos.x), y(_pos.y), z(_pos.z), normal(_normal), tangent(_tangent), u(_u), v(_v), abgr(rawrBox::Color::pack(cl)) {}
		MeshVertexData(float _x, float _y, float _z, uint32_t _normal, uint32_t _tangent, float _u, float _v, uint32_t _abgr) : x(_x), y(_y), z(_z), normal(_normal), tangent(_tangent), u(_u), v(_v), abgr(_abgr) {}
	};

	class Mesh {
	public:
		std::string name = "mesh";

		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;
		uint16_t totalVertex = 0;
		uint16_t totalIndex = 0;

		std::shared_ptr<rawrBox::TextureBase> texture = nullptr;

		std::shared_ptr<rawrBox::TextureBase> specularTexture = nullptr;
		float specularShininess = 0.f;

		std::vector<rawrBox::MeshVertexData> vertices = {};
		std::vector<uint16_t> indices = {};

		std::array<float, 16> offsetMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default
		std::array<float, 16> vertexPos = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};    // Identity matrix by default
		rawrBox::Colorf color = rawrBox::Colors::White;

		bool wireframe = false;
		uint64_t culling = BGFX_STATE_CULL_CW;
		uint64_t blending = BGFX_STATE_BLEND_ALPHA;

		Mesh() = default;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		~Mesh() {
			this->texture = nullptr;
			this->specularTexture = nullptr;

			this->vertices.clear();
			this->indices.clear();
		}

		// UTILS ----
		const std::string& getName() {
			return this->name;
		}

		void setName(const std::string& name) {
			this->name = name;
		}

		std::vector<rawrBox::MeshVertexData>& getVertices() {
			return this->vertices;
		}

		std::vector<uint16_t>& getIndices() {
			return this->indices;
		}

		void setMatrix(const std::array<float, 16>& offset) {
			this->offsetMatrix = offset;
		}

		void setTexture(std::shared_ptr<rawrBox::TextureBase> ptr) {
			this->texture = ptr;
		}

		void setWireframe(bool wireframe) {
			this->wireframe = wireframe;
		}

		void setCulling(uint64_t culling) {
			this->culling = culling;
		}

		void setBlend(uint64_t blend) {
			this->blending = blend;
		}

		void setSpecularTexture(std::shared_ptr<rawrBox::TextureBase> ptr, float shininess) {
			this->specularTexture = ptr;
			this->specularShininess = shininess;
		}

		void setColor(const rawrBox::Colorf& color) {
			this->color = color;
		}

		bool canMerge(std::shared_ptr<rawrBox::Mesh> other) {
			return this->texture == other->texture &&
			       this->color == other->color &&
			       this->wireframe == other->wireframe &&
			       this->offsetMatrix == other->offsetMatrix;
		}
	};
} // namespace rawrBox
