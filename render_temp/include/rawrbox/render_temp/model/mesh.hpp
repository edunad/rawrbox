#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render_temp/model/defs.hpp>
#include <rawrbox/render_temp/static.hpp>
#include <rawrbox/render_temp/texture/base.hpp>

#include <bgfx/bgfx.h>
#include <fmt/printf.h>

#include <cstdint>
#include <string>
#include <unordered_map>

namespace rawrbox {
	struct Skeleton;
	class LightBase;

	class Mesh {
	protected:
		bool _canOptimize = true;

		rawrbox::Vector3f _scale = {1, 1, 1};
		rawrbox::Vector3f _pos = {};
		rawrbox::Vector4f _angle = {};

	public:
		Mesh() = default;
		Mesh(const Mesh&) = default;
		Mesh(Mesh&&) = default;
		Mesh& operator=(const Mesh&) = default;
		Mesh& operator=(Mesh&&) = default;
		virtual ~Mesh() = default;

		std::string name = "mesh";

		// OFFSETS ---
		uint16_t baseVertex = 0;
		uint16_t baseIndex = 0;
		uint16_t totalVertex = 0;
		uint16_t totalIndex = 0;

		std::vector<rawrbox::VertexData> vertices = {};
		std::vector<uint16_t> indices = {};
		// -------

		// TEXTURES ---
		rawrbox::TextureBase* texture = nullptr;
		rawrbox::TextureBase* normalTexture = nullptr;

		rawrbox::TextureBase* specularTexture = nullptr;
		rawrbox::TextureBase* emissionTexture = nullptr;

		rawrbox::TextureBase* displacementTexture = nullptr;

		float specularShininess = 25.0F;
		float emissionIntensity = 1.F;
		// -------

		// RENDERING ---
		rawrbox::Matrix4x4 matrix = {};
		rawrbox::Color color = rawrbox::Colors::White();

		bool wireframe = false;
		bool lineMode = false;

		uint64_t culling = BGFX_STATE_CULL_CW;
		uint64_t blending = BGFX_STATE_BLEND_NORMAL;
		uint64_t depthTest = BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS;

		rawrbox::BBOX bbox = {};
		// --------------

		// ANIMATION ------
		rawrbox::Skeleton* skeleton = nullptr;
		// -----------------

		// LIGHTS ------
		std::vector<rawrbox::LightBase*> lights = {};
		// -----------------

		void* owner = nullptr;                                        // Eeeehhhh
		std::unordered_map<std::string, rawrbox::Vector4f> data = {}; // Other data

		// UTILS ----
		[[nodiscard]] virtual const std::string& getName() const;
		virtual void setName(const std::string& name);

		[[nodiscard]] virtual const std::vector<rawrbox::VertexData>& getVertices() const;
		[[nodiscard]] virtual const std::vector<uint16_t>& getIndices() const;
		[[nodiscard]] virtual const rawrbox::BBOX& getBBOX() const;

		[[nodiscard]] virtual bool empty() const;
		[[nodiscard]] virtual const rawrbox::Matrix4x4& getMatrix();

		[[nodiscard]] virtual const rawrbox::Vector3f& getPos() const;
		virtual void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] virtual const rawrbox::Vector4f& getAngle() const;
		virtual void setAngle(const rawrbox::Vector4f& ang);
		virtual void setEulerAngle(const rawrbox::Vector3f& ang);

		[[nodiscard]] virtual const rawrbox::Vector3f& getScale() const;
		virtual void setScale(const rawrbox::Vector3f& scale);

		template <typename B>
		B* getOwner() {
			if (this->owner == nullptr) return nullptr;
			return std::bit_cast<B*>(this->owner);
		}

		[[nodiscard]] virtual const rawrbox::TextureBase* getTexture() const;
		virtual void setTexture(rawrbox::TextureBase* ptr);

		[[nodiscard]] virtual uint16_t getAtlasID(int index = -1) const;
		virtual void setAtlasID(uint16_t atlasID, int index = -1);

		[[nodiscard]] virtual const rawrbox::TextureBase* getNormalTexture() const;
		virtual void setNormalTexture(rawrbox::TextureBase* ptr);

		[[nodiscard]] virtual const rawrbox::TextureBase* getDisplacementTexture() const;
		virtual void setDisplacementTexture(rawrbox::TextureBase* ptr, float power);

		[[nodiscard]] virtual const rawrbox::TextureBase* getEmissionTexture() const;
		virtual void setEmissionTexture(rawrbox::TextureBase* ptr, float intensity);

		[[nodiscard]] virtual const rawrbox::TextureBase* getSpecularTexture() const;
		virtual void setSpecularTexture(rawrbox::TextureBase* ptr, float shininess);

		virtual void setVertexSnap(float power = 2.F);

		virtual void setWireframe(bool wireframe);

		virtual void setCulling(uint64_t culling);

		virtual void setDepthTest(uint64_t depthTest);

		virtual void setBlend(uint64_t blend);

		virtual void setRecieveDecals(bool status);

		[[nodiscard]] virtual uint32_t getId(int index = -1) const;
		virtual void setId(uint32_t id, int index = -1);

		virtual void setColor(const rawrbox::Color& color);

		virtual void addData(const std::string& id, rawrbox::Vector4f data);
		[[nodiscard]] virtual const rawrbox::Vector4f& getData(const std::string& id) const;
		[[nodiscard]] virtual bool hasData(const std::string& id) const;

		[[nodiscard]] virtual rawrbox::Skeleton* getSkeleton() const;

		virtual void clear();

		virtual void merge(const rawrbox::Mesh& other);
		virtual void rotateVertices(float rad, rawrbox::Vector3f axis = {0, 1, 0});

		virtual void setOptimizable(bool status);
		[[nodiscard]] virtual bool canOptimize(const rawrbox::Mesh& other) const;
	};
} // namespace rawrbox
