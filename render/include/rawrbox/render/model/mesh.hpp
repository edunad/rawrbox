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
	private:
		bool _canOptimize = true;

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
		rawrbox::Color specularColor = rawrbox::Colors::Black;

		std::shared_ptr<rawrbox::TextureBase> emissionTexture = nullptr;
		rawrbox::Color emissionColor = rawrbox::Colors::Black;

		float specularShininess = 25.0F;
		float emissionIntensity = 1.F;
		// -------

		// RENDERING ---
		rawrbox::Matrix4x4 offsetMatrix = {};
		rawrbox::Matrix4x4 vertexPos = {};

		rawrbox::Color color = rawrbox::Colors::White;

		bool wireframe = false;
		bool lineMode = false;

		uint64_t culling = BGFX_STATE_CULL_CW;
		uint64_t blending = BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		rawrbox::BBOX bbox = {};
		// --------------

		std::shared_ptr<Skeleton> skeleton = nullptr;
		std::unordered_map<std::string, rawrbox::Vector4f> data = {};

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
		[[nodiscard]] const std::string& getName() const {
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

		[[nodiscard]] const std::shared_ptr<rawrbox::TextureBase> getTexture() const { return this->texture; }
		void setTexture(std::shared_ptr<rawrbox::TextureBase> ptr) {
			this->texture = ptr;
		}

		[[nodiscard]] const std::shared_ptr<rawrbox::TextureBase> getEmissionTexture() const { return this->emissionTexture; }
		void setEmissionTexture(std::shared_ptr<rawrbox::TextureBase> ptr, float intensity) {
			this->emissionTexture = ptr;
			this->emissionIntensity = intensity;
		}

		[[nodiscard]] const std::shared_ptr<rawrbox::TextureBase> getSpecularTexture() const { return this->specularTexture; }
		void setSpecularTexture(std::shared_ptr<rawrbox::TextureBase> ptr, float shininess) {
			this->specularTexture = ptr;
			this->specularShininess = shininess;
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

		void setColor(const rawrbox::Color& color) {
			this->color = color;
		}

		void setSpecularColor(const rawrbox::Color& color) {
			this->specularColor = color;
		}

		void setEmissionColor(const rawrbox::Color& color) {
			this->emissionColor = color;
		}

		void addData(const std::string& id, rawrbox::Vector4f data) { // BGFX shaders only accept vec4, so.. yea
			this->data[id] = data;
		}

		rawrbox::Vector4f getData(const std::string& id) {
			auto fnd = this->data.find(id);
			if (fnd == this->data.end()) throw std::runtime_error(fmt::format("[RawrBox-Mesh] Data '{}' not found", id));
			return fnd->second;
		}

		bool hasData(const std::string& id) {
			return this->data.find(id) != this->data.end();
		}

		void merge(std::shared_ptr<rawrbox::Mesh<T>> other) {
			for (uint16_t i : other->indices)
				this->indices.push_back(this->totalVertex + i);
			this->vertices.insert(this->vertices.end(), other->vertices.begin(), other->vertices.end());

			this->totalVertex += other->totalVertex;
			this->totalIndex += other->totalIndex;
		}

		void clear() {
			this->vertices.clear();
			this->indices.clear();

			this->totalIndex = 0;
			this->totalVertex = 0;
			this->baseIndex = 0;
			this->baseVertex = 0;
		}

		void setOptimizable(bool status) { this->_canOptimize = status; }
		bool canOptimize(std::shared_ptr<rawrbox::Mesh<T>> other) {
			if (this->skeleton != nullptr || !this->_canOptimize) return false;
			return this->texture == other->texture &&
			       this->color == other->color &&
			       this->wireframe == other->wireframe &&
			       this->offsetMatrix == other->offsetMatrix;
		}
	};
} // namespace rawrbox
