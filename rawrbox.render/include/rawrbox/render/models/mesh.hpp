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

	struct MeshTextures {
	public:
		rawrbox::TextureBase* texture = nullptr;
		rawrbox::TextureBase* normal = nullptr;
		rawrbox::TextureBase* roughtMetal = nullptr;
		rawrbox::TextureBase* emission = nullptr;

		rawrbox::TextureBase* displacement = nullptr;

		float roughnessFactor = 1.0F;
		float metalnessFactor = 1.0F;
		float specularFactor = 0.5F;
		float emissionFactor = 1.0F;

		float displacementPower = 1.F;

		[[nodiscard]] rawrbox::Vector4f getData() const {
			return {roughnessFactor, metalnessFactor, specularFactor, emissionFactor};
		}

		[[nodiscard]] rawrbox::Vector4_t<uint32_t> getPixelIDs() const {
			auto* base = texture == nullptr ? rawrbox::WHITE_TEXTURE.get() : texture;
			auto* norm = normal == nullptr ? rawrbox::NORMAL_TEXTURE.get() : normal;
			auto* metR = roughtMetal == nullptr ? rawrbox::BLACK_TEXTURE.get() : roughtMetal;
			auto* em = emission == nullptr ? rawrbox::BLACK_TEXTURE.get() : emission;

			return {base->getTextureID(), norm->getTextureID(), metR->getTextureID(), em->getTextureID()};
		}

		[[nodiscard]] bool canMerge(const rawrbox::MeshTextures& other) const {
			return this->roughnessFactor == other.roughnessFactor && this->metalnessFactor == other.metalnessFactor && this->specularFactor == other.specularFactor && this->emissionFactor == other.emissionFactor;
		}

		bool operator==(const rawrbox::MeshTextures& other) const { return this->texture == other.texture && this->normal == other.normal && this->specularFactor == other.specularFactor && this->roughtMetal == other.roughtMetal && this->emission == other.emission; }
		bool operator!=(const rawrbox::MeshTextures& other) const { return !operator==(other); }
	};

	template <typename T = rawrbox::VertexData>
		requires(std::derived_from<T, rawrbox::VertexData>)
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
		rawrbox::MeshTextures textures = {};
		// -------

		// OTHER ---
		float vertexSnapPower = 0.F;
		bool billboard = false;
		// ---------

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

		void* owner = nullptr;                            // Eeeehhhh
		std::unordered_map<std::string, float> data = {}; // Other data

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

		[[nodiscard]] virtual const rawrbox::TextureBase* getTexture() const { return this->textures.texture; }
		virtual void setTexture(rawrbox::TextureBase* ptr) {
			this->textures.texture = ptr;
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getNormalTexture() const { return this->textures.normal; }
		virtual void setNormalTexture(rawrbox::TextureBase* ptr) {
			this->textures.normal = ptr;
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getDisplacementTexture() const { return this->textures.displacement; }
		virtual void setDisplacementTexture(rawrbox::TextureBase* ptr, float power = 1.F) {
			this->textures.displacement = ptr;
			this->textures.displacementPower = power;

			this->setOptimizable(false);
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getEmissionTexture() const { return this->textures.emission; }
		virtual void setEmissionTexture(rawrbox::TextureBase* ptr, float factor = 1.0F) {
			this->textures.emission = ptr;
			this->textures.emissionFactor = factor;
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getRoughtMetalTexture() const { return this->textures.roughtMetal; }
		virtual void setRoughtMetalTexture(rawrbox::TextureBase* ptr, float roughness = 0.5F, float metalness = 0.5F) {
			this->textures.roughtMetal = ptr;
			this->textures.roughnessFactor = roughness;
			this->textures.metalnessFactor = metalness;
		}

		virtual void setSpecularFactor(float spec) {
			this->textures.specularFactor = spec;
		}

		virtual void setBillboard(bool set) {
			this->billboard = set;
		}

		virtual void setVertexSnap(float power = 2.F) {
			this->vertexSnapPower = power;
		}

		virtual void setWireframe(bool _wireframe) {
			this->wireframe = _wireframe;
		}

		virtual void setCulling(Diligent::CULL_MODE _culling) {
			this->culling = _culling;
		}

		[[nodiscard]] virtual uint32_t getId(int /*index*/ = -1) const { return 0; }
		virtual void setId(uint32_t /*id*/, int /*index*/ = -1) {}

		[[nodiscard]] virtual const rawrbox::Color& getColor() const { return this->color; }
		virtual void setColor(const rawrbox::Color& _color) {
			this->color = _color;
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
			std::transform(other.indices.begin(), other.indices.end(), std::back_inserter(this->indices), [this](const uint16_t& val) { return static_cast<uint16_t>(this->totalVertex + val); });
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

			return this->textures == other.textures && // TODO: Replace with canMerge and pass textureID down to vertex?
			       this->color == other.color &&
			       this->wireframe == other.wireframe &&
			       this->lineMode == other.lineMode &&
			       this->matrix == other.matrix;
		}
	};
} // namespace rawrbox
