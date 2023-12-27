#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/models/vertex.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/textures/base.hpp>

#include <RasterizerState.h>
#include <fmt/printf.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace rawrbox {
	struct Skeleton;
	class LightBase;

	template <typename T = VertexData>
	class Mesh {
		static constexpr uint16_t MAX_VERTICES = 16000;
		static constexpr uint16_t MAX_INDICES = 16000;

	protected:
		bool _canOptimize = true;

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

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
		rawrbox::TextureBase* texture = nullptr;
		rawrbox::TextureBase* normalTexture = nullptr;

		rawrbox::TextureBase* emissionTexture = nullptr;
		rawrbox::TextureBase* roughtMetalTexture = nullptr;

		rawrbox::TextureBase* displacementTexture = nullptr;

		float roughnessFactor = 1.0F;
		float metalnessFactor = 1.0F;
		float specularFactor = 0.5F;
		float emissionFactor = 1.0F;
		// -------

		// RENDERING ---
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Color color = rawrbox::Colors::White();

		bool wireframe = false;
		bool lineMode = false;
		bool alphaBlend = false;

		Diligent::CULL_MODE culling = Diligent::CULL_MODE_FRONT;
		rawrbox::BBOX bbox = {};
		// --------------

		// ANIMATION ------
		rawrbox::Skeleton* skeleton = nullptr;
		std::array<rawrbox::Matrix4x4, rawrbox::MAX_BONES_PER_MODEL> boneTransforms = {};
		// -----------------

		// LIGHTS ------
		std::vector<rawrbox::LightBase*> lights = {};
		// -----------------

		void* owner = nullptr;                                        // Eeeehhhh
		std::unordered_map<std::string, rawrbox::Vector4f> data = {}; // Other data

		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) noexcept = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh& operator=(Mesh&&) noexcept = default;
		virtual ~Mesh() = default;

		// UTILS ----
		[[nodiscard]] virtual const std::string& getName() const {
			return this->name;
		}

		virtual void setName(const std::string& _name) {
			this->name = _name;
		}

		[[nodiscard]] virtual const std::vector<T>& getVertices() const {
			return this->vertices;
		}

		[[nodiscard]] virtual const std::vector<uint16_t>& getIndices() const {
			return this->indices;
		}

		[[nodiscard]] virtual const rawrbox::BBOX& getBBOX() const {
			return this->bbox;
		}

		[[nodiscard]] virtual bool empty() const {
			return this->indices.empty() || this->vertices.empty();
		}
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix() { return this->matrix; }

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const { return this->_pos; }
		virtual void setPos(const rawrbox::Vector3f& pos) {
			this->_pos = pos;
			this->matrix.SRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const { return this->_angle; }
		virtual void setAngle(const rawrbox::Vector4f& ang) {
			this->_angle = ang;
			this->matrix.SRT(this->_scale, this->_angle, this->_pos);
		}

		virtual void setEulerAngle(const rawrbox::Vector3f& ang) {
			this->_angle = rawrbox::Vector4f::toQuat(ang);
			this->matrix.SRT(this->_scale, this->_angle, this->_pos);
		}

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const { return this->_scale; }
		virtual void setScale(const rawrbox::Vector3f& scale) {
			this->_scale = scale;
			this->matrix.SRT(this->_scale, this->_angle, this->_pos);
		}

		virtual void setTransparentBlending(bool _transparent) { this->alphaBlend = _transparent; }

		template <typename B>
		B* getOwner() {
			if (this->owner == nullptr) return nullptr;
			return std::bit_cast<B*>(this->owner);
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getTexture() const { return this->texture; }
		virtual void setTexture(rawrbox::TextureBase* ptr) {
			this->texture = ptr;
		}

		[[nodiscard]] virtual uint16_t getAtlasID(int index = -1) const {
			if (this->vertices.empty()) return 0;
			if (index < 0) return static_cast<uint16_t>(this->vertices.front().uv.z);

			return static_cast<uint16_t>(this->vertices[std::clamp(index, 0, static_cast<int>(this->vertices.size() - 1))].uv.z);
		}

		virtual void setAtlasID(uint16_t _atlasId, int index = -1) {
			auto vSize = static_cast<int>(this->vertices.size());
			for (int i = 0; i < vSize; i++) {
				if (index != -1 && i == index) {
					this->vertices[i].setAtlasId(_atlasId);
					break;
				}

				this->vertices[i].setAtlasId(_atlasId);
			}
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getNormalTexture() const { return this->normalTexture; }
		virtual void setNormalTexture(rawrbox::TextureBase* ptr) { this->normalTexture = ptr; }

		[[nodiscard]] virtual const rawrbox::TextureBase* getDisplacementTexture() const { return this->displacementTexture; }
		virtual void setDisplacementTexture(rawrbox::TextureBase* ptr, float power = 1.F) {
			this->displacementTexture = ptr;
			this->addData("displacement_strength", {power, 0, 0, 0});
			this->setOptimizable(false);
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getEmissionTexture() const { return this->emissionTexture; }
		virtual void setEmissionTexture(rawrbox::TextureBase* ptr, float factor = 1.0F) {
			this->emissionTexture = ptr;
			this->emissionFactor = factor;
		}

		virtual void setRoughtnessMetalness(rawrbox::TextureBase* ptr, float roughness = 0.5F, float metalness = 0.5F) {
			this->roughtMetalTexture = ptr;
			this->roughnessFactor = roughness;
			this->metalnessFactor = metalness;
		}

		virtual void setSpecularFactor(float spec) {
			this->specularFactor = spec;
		}

		virtual void setVertexSnap(float power = 2.F) {
			this->addData("vertex_snap", {power, 0, 0, 0});
			this->setOptimizable(false);
		}

		virtual void setWireframe(bool _wireframe) {
			this->wireframe = _wireframe;
		}

		virtual void setCulling(Diligent::CULL_MODE _culling) {
			this->culling = _culling;
		}

		virtual void setRecieveDecals(bool decals) {
			this->addData("mask", {decals ? 1.0F : 0.0F, 0, 0, 0});
		}

		[[nodiscard]] virtual uint32_t getId(int /*index*/ = -1) const { return 0; }
		virtual void setId(uint32_t id, int index = -1) {}

		[[nodiscard]] virtual const rawrbox::Color& getColor() const { return this->color; }
		virtual void setColor(const rawrbox::Color& _color) {
			this->color = _color;
		}

		virtual void addData(const std::string& id, rawrbox::Vector4f _data) {
			this->data[id] = _data;
		}

		[[nodiscard]] virtual const rawrbox::Vector4f& getData(const std::string& id) const {
			auto fnd = this->data.find(id);
			if (fnd == this->data.end()) throw std::runtime_error(fmt::format("[RawrBox-Mesh] Data '{}' not found", id));
			return fnd->second;
		}

		[[nodiscard]] virtual bool hasData(const std::string& id) const {
			return this->data.find(id) != this->data.end();
		}

		[[nodiscard]] virtual rawrbox::Skeleton* getSkeleton() const {
			return this->skeleton;
		}

		virtual void clear() {
			this->vertices.clear();
			this->indices.clear();

			this->totalIndex = 0;
			this->totalVertex = 0;
			this->baseIndex = 0;
			this->baseVertex = 0;
		}

		virtual void merge(const rawrbox::Mesh<T>& other) {
			std::transform(other.indices.begin(), other.indices.end(), std::back_inserter(this->indices), [this](uint16_t val) { return this->totalVertex + val; });
			this->vertices.insert(this->vertices.end(), other.vertices.begin(), other.vertices.end());

			this->totalVertex = static_cast<uint16_t>(this->vertices.size());
			this->totalIndex = static_cast<uint16_t>(this->indices.size());

			this->bbox.combine(other.bbox);
		}

		virtual void rotateVertices(float rad, rawrbox::Vector3f axis = {0, 1, 0}) {
			for (auto& v : vertices) {
				v.position = v.position.rotateAroundOrigin(axis, rad);
			}
		}

		virtual void setOptimizable(bool status) { this->_canOptimize = status; }
		[[nodiscard]] virtual bool canOptimize(const rawrbox::Mesh<T>& other) const {
			if (!this->_canOptimize || !other._canOptimize) return false;

			if (this->vertices.size() + other.vertices.size() >= MAX_VERTICES) return false; // Max vertice limit
			if (this->indices.size() + other.indices.size() >= MAX_INDICES) return false;    // Max indice limit

			return this->texture == other.texture &&
			       this->emissionTexture == other.emissionTexture &&
			       this->roughtMetalTexture == other.roughtMetalTexture &&
			       this->metalnessFactor == other.metalnessFactor &&
			       this->roughnessFactor == other.roughnessFactor &&
			       this->color == other.color &&
			       this->wireframe == other.wireframe &&
			       this->lineMode == other.lineMode &&
			       this->matrix == other.matrix;
		}
	};
} // namespace rawrbox
