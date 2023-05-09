#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/texture/base.hpp>

#include <bgfx/bgfx.h>
#include <fmt/printf.h>

#include <array>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace rawrbox {

	struct BBOX {
	public:
		rawrbox::Vector3f m_min = {};
		rawrbox::Vector3f m_max = {};
		rawrbox::Vector3f m_size = {};

		[[nodiscard]] bool isEmpty() const {
			return this->m_size == 0;
		}

		[[nodiscard]] const rawrbox::Vector3f& size() const {
			return this->m_size;
		}

		void combine(const rawrbox::BBOX& b) {
			this->m_min = {std::min(this->m_min.x, b.m_min.x), std::min(this->m_min.y, b.m_min.y), std::min(this->m_min.z, b.m_min.z)};
			this->m_max = {std::max(this->m_max.x, b.m_max.x), std::max(this->m_max.y, b.m_max.y), std::max(this->m_max.z, b.m_max.z)};

			this->m_size = m_min.abs() + m_max.abs();
		}

		bool operator==(const rawrbox::BBOX& other) const { return this->m_size == other.m_size; }
		bool operator!=(const rawrbox::BBOX& other) const { return !operator==(other); }
	};

	struct VertexData;
	struct Skeleton;

	template <typename T = VertexData>
	class Mesh {
	public:
		std::string name = "mesh";

		// OFFSETS ---
		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;
		uint16_t totalVertex = 0;
		uint16_t totalIndex = 0;

		std::vector<T> vertices = {};
		std::vector<uint16_t> indices = {};
		// -------

		// TEXTURES ---
		std::shared_ptr<rawrbox::TextureBase> texture = nullptr;
		std::shared_ptr<rawrbox::TextureBase> specularTexture = nullptr;
		float specularShininess = 1.0F;
		// -------

		// RENDERING ---
		rawrbox::Matrix4x4 offsetMatrix = {};
		rawrbox::Matrix4x4 vertexPos = {};
		rawrbox::Color color = rawrbox::Colors::White;

		bool wireframe = false;
		uint64_t culling = BGFX_STATE_CULL_CW;
		uint64_t blending = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		rawrbox::BBOX bbox = {};
		// --------------

		std::shared_ptr<Mesh<T>> parent = nullptr;
		std::shared_ptr<Skeleton> skeleton = nullptr;

		std::unordered_map<std::string, rawrbox::Vector3f> data = {};

		Mesh() = default;
		Mesh(Mesh&&) = delete;
		Mesh& operator=(Mesh&&) = delete;
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;

		virtual ~Mesh() {
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

		std::vector<T>& getVertices() {
			return this->vertices;
		}

		std::vector<uint16_t>& getIndices() {
			return this->indices;
		}

		rawrbox::BBOX& getBBOX() {
			return this->bbox;
		}

		void setMatrix(const rawrbox::Matrix4x4& offset) {
			this->offsetMatrix = offset;
		}

		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->offsetMatrix.translate(pos);
		}

		void setTexture(std::shared_ptr<rawrbox::TextureBase> ptr) {
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

		void setSpecularTexture(std::shared_ptr<rawrbox::TextureBase> ptr, float shininess) {
			this->specularTexture = ptr;
			this->specularShininess = shininess;
		}

		void setColor(const rawrbox::Color& color) {
			this->color = color;
		}

		void addData(const std::string& id, rawrbox::Vector3f data) { // BGFX shaders only accept vec4, so.. yea
			if (this->hasData(id)) throw std::runtime_error(fmt::format("[RawrBox-Mesh] Data '{}' already added", id));
			this->data[id] = data;
		}

		rawrbox::Vector3f getData(const std::string& id) {
			auto fnd = this->data.find(id);
			if (fnd == this->data.end()) throw std::runtime_error(fmt::format("[RawrBox-Mesh] Data '{}' not found", id));
			return fnd->second;
		}

		bool hasData(const std::string& id) {
			return this->data.find(id) != this->data.end();
		}

		void merge(std::shared_ptr<rawrbox::Mesh<T>> other) {
			for (uint16_t i : other->indices)
				this->indices.push_back(static_cast<uint16_t>(this->vertices.size()) + i);

			this->vertices.insert(this->vertices.end(), other->vertices.begin(), other->vertices.end());
			this->totalVertex = static_cast<uint16_t>(this->vertices.size());
			this->totalIndex = static_cast<uint16_t>(this->indices.size());
		}

		bool canMerge(std::shared_ptr<rawrbox::Mesh<T>> other) {
			return this->texture == other->texture &&
			       this->color == other->color &&
			       this->wireframe == other->wireframe &&
			       this->offsetMatrix == other->offsetMatrix;
		}
	};
} // namespace rawrbox
