#pragma once

#include <rawrbox/math/bbox.hpp>
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
	struct VertexData;
	struct Skeleton;

	class LightBase;

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
		std::shared_ptr<rawrbox::TextureBase> opacityTexture = nullptr;

		std::shared_ptr<rawrbox::TextureBase> specularTexture = nullptr;
		rawrbox::Color specularColor = rawrbox::Colors::White;

		std::shared_ptr<rawrbox::TextureBase> emissionTexture = nullptr;
		rawrbox::Color emissionColor = rawrbox::Colors::White;

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
		uint64_t blending = BGFX_STATE_BLEND_NORMAL;
		uint64_t depthTest = BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS;

		rawrbox::BBOX bbox = {};
		// --------------

		// ANIMATION ------
		std::weak_ptr<rawrbox::Skeleton> skeleton;
		// -----------------

		// LIGHTS ------
		std::vector<std::weak_ptr<rawrbox::LightBase>> lights = {};
		// -----------------

		void* owner = nullptr; // Eeeehhhh
		std::unordered_map<std::string, rawrbox::Vector4f> data = {};

		Mesh() = default;
		virtual ~Mesh() {
			this->texture = nullptr;
			this->specularTexture = nullptr;
			this->emissionTexture = nullptr;
			this->opacityTexture = nullptr;

			this->owner = nullptr;
			this->skeleton.reset();

			this->lights.clear();
			this->data.clear();
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

		[[nodiscard]] const std::vector<T>& getVertices() const {
			return this->vertices;
		}

		[[nodiscard]] const std::vector<uint16_t>& getIndices() const {
			return this->indices;
		}

		[[nodiscard]] const rawrbox::BBOX& getBBOX() const {
			return this->bbox;
		}

		[[nodiscard]] const rawrbox::Vector3f getPos() const {
			return {this->offsetMatrix[12], this->offsetMatrix[13], this->offsetMatrix[14]};
		}

		void setMatrix(const rawrbox::Matrix4x4& offset) {
			this->offsetMatrix = offset;
		}

		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->offsetMatrix.translate(pos);
		}

		template <typename B>
		B* getOwner() {
			if (this->owner == nullptr) return nullptr;
			return std::bit_cast<B*>(this->owner);
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

		[[nodiscard]] const std::shared_ptr<rawrbox::TextureBase> getOpacityTexture() const { return this->opacityTexture; }
		void setOpacityTexture(std::shared_ptr<rawrbox::TextureBase> ptr) {
			this->opacityTexture = ptr;
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

		void setDepthTest(uint64_t depthTest) {
			this->depthTest = depthTest;
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
			if (!this->_canOptimize || !other->_canOptimize) return false;

			return this->texture == other->texture &&
			       this->color == other->color &&
			       this->wireframe == other->wireframe &&
			       this->offsetMatrix == other->offsetMatrix;
		}
	};
} // namespace rawrbox
