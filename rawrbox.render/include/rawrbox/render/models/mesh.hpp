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

namespace ozz {
	namespace animation {
		class Skeleton;
	} // namespace animation
} // namespace ozz

namespace rawrbox {
	class LightBase;

	namespace MeshBilldboard {
		const uint32_t X = 1 << 1;
		const uint32_t Y = 1 << 2;
		const uint32_t ALL = X | Y;
	}; // namespace MeshBilldboard

	struct MeshTextures {
	public:
		rawrbox::TextureBase* texture = nullptr;
		rawrbox::TextureBase* normal = nullptr;
		rawrbox::TextureBase* roughtMetal = nullptr;
		rawrbox::TextureBase* emission = nullptr;

		float roughnessFactor = 1.0F;
		float metalnessFactor = 1.0F;
		float specularFactor = 0.5F;
		float emissionFactor = 1.0F;

		float alphaCutoff = 0.5F;

		[[nodiscard]] rawrbox::Vector4f getTextureData() const {
			return {roughnessFactor, metalnessFactor, specularFactor, emissionFactor};
		}

		[[nodiscard]] rawrbox::Vector4f getData() const {
			return {alphaCutoff, 0.F, 0.F, 0.F};
		}

		[[nodiscard]] rawrbox::Vector4_t<uint32_t> getTextureIDs() const {
			auto* base = texture == nullptr ? rawrbox::WHITE_TEXTURE.get() : texture;
			auto* norm = normal == nullptr ? rawrbox::NORMAL_TEXTURE.get() : normal;
			auto* metR = roughtMetal == nullptr ? rawrbox::BLACK_TEXTURE.get() : roughtMetal;
			auto* em = emission == nullptr ? rawrbox::BLACK_TEXTURE.get() : emission;

			return {base->getTextureID(), norm->getTextureID(), metR->getTextureID(), em->getTextureID()};
		}

		[[nodiscard]] bool canMerge(const rawrbox::MeshTextures& other) const {
			return this->roughnessFactor == other.roughnessFactor && this->metalnessFactor == other.metalnessFactor && this->specularFactor == other.specularFactor && this->emissionFactor == other.emissionFactor && this->alphaCutoff == other.alphaCutoff;
		}

		bool operator==(const rawrbox::MeshTextures& other) const { return this->texture == other.texture && this->normal == other.normal && this->specularFactor == other.specularFactor && this->roughtMetal == other.roughtMetal && this->emission == other.emission; }
		bool operator!=(const rawrbox::MeshTextures& other) const { return !operator==(other); }
	};

	struct MeshData { // Aka data for vertex shader
	public:
		float vertexSnapPower = 0.F;
		uint32_t slice = 0;
		uint32_t billboard = 0;

		// Displacement ---
		rawrbox::TextureBase* displacement = nullptr;
		float displacementPower = 1.F;
		// --------------

		bool operator==(const rawrbox::MeshData& other) const { return this->vertexSnapPower == other.vertexSnapPower && this->billboard == other.billboard && this->slice == other.slice && this->displacement == other.displacement && this->displacementPower == other.displacementPower; }
		bool operator!=(const rawrbox::MeshData& other) const { return !operator==(other); }
	};

	template <typename T = rawrbox::VertexUVData>
		requires(std::derived_from<T, rawrbox::VertexData>)
	class Mesh {

	protected:
		bool _canMerge = true;
		bool _wireframe = false;
		bool _lineMode = false;
		bool _transparent = false;

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

	public:
		std::string name = "mesh";

		// OFFSETS ---
		uint32_t baseVertex = 0;
		uint32_t baseIndex = 0;
		uint32_t totalVertex = 0;
		uint32_t totalIndex = 0;

		std::vector<T> vertices = {};
		std::vector<uint32_t> indices = {};
		// -------

		// TEXTURES ---
		rawrbox::MeshTextures textures = {};
		// -------

		// OTHER ---
		rawrbox::MeshData data = {};
		uint32_t meshID = 0x00000000;
		// ---------

		// RENDERING ---
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Color color = rawrbox::Colors::White();

		Diligent::CULL_MODE culling = Diligent::CULL_MODE_FRONT;
		rawrbox::BBOX bbox = {};
		// --------------

		// ANIMATION ------
		ozz::animation::Skeleton* skeleton = nullptr;
		std::array<rawrbox::Matrix4x4, RB_RENDER_MAX_BONES_PER_MODEL> boneTransforms = {};
		// -----------------

		// LIGHTS ------
		std::vector<rawrbox::LightBase*> lights = {};
		// -----------------

		void* owner = nullptr; // Eeeehhhh

		Mesh(size_t vertexSize = 0, size_t indexSize = 0) {
			if (vertexSize != 0) this->vertices.resize(vertexSize);
			if (indexSize != 0) this->indices.resize(indexSize);
		};

		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) noexcept = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh& operator=(Mesh&&) noexcept = default;
		virtual ~Mesh() = default;

		// UTILS ----
		[[nodiscard]] virtual const std::string& getName() const { return this->name; }
		virtual void setName(const std::string& _name) { this->name = _name; }

		[[nodiscard]] virtual bool getWireframe() const { return this->_wireframe; }
		virtual void setWireframe(bool wireframe) { this->_wireframe = wireframe; }

		[[nodiscard]] virtual bool getLineMode() const { return this->_lineMode; }
		virtual void setLineMode(bool line) { this->_lineMode = line; }

		virtual void setAlphaCutoff(float cutoff) { this->textures.alphaCutoff = cutoff; }
		virtual void setTransparent(bool transparent) { this->_transparent = transparent; }
		[[nodiscard]] virtual bool isTransparent() const { return this->_transparent; }

		[[nodiscard]] virtual const std::vector<T>& getVertices() const { return this->vertices; }
		[[nodiscard]] virtual const std::vector<uint32_t>& getIndices() const { return this->indices; }

		[[nodiscard]] virtual rawrbox::BBOX getBBOX() const { return this->bbox * this->_scale; }

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

		template <typename B>
		B* getOwner() {
			if (this->owner == nullptr) return nullptr;
			return std::bit_cast<B*>(this->owner);
		}

		[[nodiscard]] virtual uint32_t getSlice() const { return this->data.slice; }
		virtual void setSlice(uint32_t slice) { this->data.slice = slice; }

		[[nodiscard]] virtual const rawrbox::TextureBase* getTexture() const { return this->textures.texture; }
		virtual void setTexture(rawrbox::TextureBase* ptr) {
			this->textures.texture = ptr;
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getNormalTexture() const { return this->textures.normal; }
		virtual void setNormalTexture(rawrbox::TextureBase* ptr) {
			this->textures.normal = ptr;
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getDisplacementTexture() const { return this->data.displacement; }
		virtual void setDisplacementTexture(rawrbox::TextureBase* ptr, float power = 1.F) {
			this->data.displacement = ptr;
			this->data.displacementPower = power;
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

		virtual void setBillboard(uint32_t set) {
			this->data.billboard = set;
		}

		virtual void setVertexSnap(float power = 2.F) {
			this->data.vertexSnapPower = power;
		}

		virtual void setCulling(Diligent::CULL_MODE _culling) {
			this->culling = _culling;
		}

		[[nodiscard]] virtual uint32_t getID() const { return this->meshID; }
		virtual void setID(uint32_t id) { this->meshID = (id << 8) | 0xFF; }

		[[nodiscard]] virtual const rawrbox::Color& getColor() const { return this->color; }
		virtual void setColor(const rawrbox::Color& _color) {
			this->color = _color;
		}

		virtual void repeatUV(uint32_t slices) {
			if (this->empty()) return;

			if constexpr (supportsUVs<T>) {
				for (auto& vert : this->vertices) {
					vert.uv.x *= slices;
					vert.uv.y *= slices;
				}
			}
		}

		[[nodiscard]] virtual ozz::animation::Skeleton* getSkeleton() const {
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
			std::transform(other.indices.begin(), other.indices.end(), std::back_inserter(this->indices), [this](const uint32_t& val) { return this->totalVertex + val; });
			this->vertices.insert(this->vertices.end(), other.vertices.begin(), other.vertices.end());

			this->totalVertex = static_cast<uint32_t>(this->vertices.size());
			this->totalIndex = static_cast<uint32_t>(this->indices.size());

			this->bbox.combine(other.bbox);
		}

		virtual void setMergeable(bool status) { this->_canMerge = status; }
		virtual bool isMergeable() const { return this->_canMerge; }

		[[nodiscard]] virtual bool canMerge(const rawrbox::Mesh<T>& other) const {
			if (!this->_canMerge || !other._canMerge) return false;

			if (this->vertices.size() + other.vertices.size() >= RB_RENDER_MAX_VERTICES) return false; // Max vertice limit
			if (this->indices.size() + other.indices.size() >= RB_RENDER_MAX_INDICES) return false;    // Max indice limit

			return this->textures == other.textures && // TODO: Replace with canMerge and pass textureID down to vertex?
			       this->color == other.color &&
			       this->meshID == other.meshID &&
			       this->data == other.data &&
			       this->_wireframe == other._wireframe &&
			       this->_lineMode == other._lineMode &&
			       this->_transparent == other._transparent &&
			       this->matrix == other.matrix;
		}
	};
} // namespace rawrbox
