#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/quaternion.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <fmt/printf.h>

#include <array>
#include <cstdint>
#include <map>
#include <memory>

namespace rawrBox {

	struct MeshVertexData {
		std::array<float, 3> position = {0, 0, 0};
		std::array<uint32_t, 2> normal = {0, 0}; // normal, tangent
		std::array<float, 2> uv = {0, 0};

		uint32_t abgr = 0xFFFFFFFF;

		std::array<uint8_t, rawrBox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrBox::MAX_BONES_PER_VERTEX> bone_weights = {};

		MeshVertexData() = default;
		MeshVertexData(const rawrBox::Vector3f& _pos, uint32_t _normal, uint32_t _tangent,
		    float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : position({_pos.x, _pos.y, _pos.z}), normal({_normal, _tangent}), uv({_u, _v}), abgr(rawrBox::Color::pack(cl)) {}

		int index = 0;
		void addBoneData(uint8_t boneId, float weight) {
			if (index < rawrBox::MAX_BONES_PER_VERTEX) {
				this->bone_indices[index] = boneId;
				this->bone_weights[index] = weight;

				index++;
			} else {
				// find the bone with the smallest weight
				int minIndex = 0;
				float minWeight = this->bone_weights[0];
				for (int i = 1; i < rawrBox::MAX_BONES_PER_VERTEX; i++) {
					if (this->bone_weights[i] < minWeight) {
						minIndex = i;
						minWeight = this->bone_weights[i];
					}
				}

				// replace with new bone if the new bone has greater weight
				if (weight > minWeight) {
					this->bone_indices[minIndex] = boneId;
					this->bone_weights[minIndex] = weight;
				}
			}
		}
	};

	struct Skeleton;
	class Mesh {
	public:
		std::string name = "mesh";

		// OFFSETS ---
		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;
		uint16_t totalVertex = 0;
		uint16_t totalIndex = 0;

		std::vector<rawrBox::MeshVertexData> vertices = {};
		std::vector<uint16_t> indices = {};
		// -------

		// TEXTURES ---
		std::shared_ptr<rawrBox::TextureBase> texture = nullptr;
		std::shared_ptr<rawrBox::TextureBase> specularTexture = nullptr;
		float specularShininess = 1.0F;
		// -------

		// RENDERING ---
		std::array<float, 16> offsetMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; // Identity matrix by default
		std::array<float, 16> vertexPos = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};    // Identity matrix by default
		rawrBox::Colorf color = rawrBox::Colors::White;

		bool wireframe = false;
		uint64_t culling = BGFX_STATE_CULL_CW;
		uint64_t blending = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		// --------------

		std::shared_ptr<Mesh> parent = nullptr;
		std::shared_ptr<Skeleton> skeleton = nullptr;

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
