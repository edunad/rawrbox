#pragma once

#include <rawrbox/math/bbox.hpp>
#include <rawrbox/math/color.hpp>
#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/utils/math.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/model/defs.hpp>
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
	struct Skeleton;
	class LightBase;

	class Mesh {
	private:
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

		std::vector<rawrbox::VertexData> vertices = {};
		std::vector<uint16_t> indices = {};
		// -------

		// TEXTURES ---
		rawrbox::TextureBase* texture = nullptr;
		rawrbox::TextureBase* normalTexture = nullptr;
		rawrbox::TextureBase* opacityTexture = nullptr;

		rawrbox::TextureBase* specularTexture = nullptr;
		rawrbox::Color specularColor = rawrbox::Colors::White;

		rawrbox::TextureBase* emissionTexture = nullptr;
		rawrbox::Color emissionColor = rawrbox::Colors::White;

		float specularShininess = 25.0F;
		float emissionIntensity = 1.F;
		// -------

		// RENDERING ---
		rawrbox::Matrix4x4 matrix = {};
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
		rawrbox::Skeleton* skeleton = nullptr;
		// -----------------

		// LIGHTS ------
		std::vector<rawrbox::LightBase*> lights = {};
		// -----------------

		void* owner = nullptr;                                        // Eeeehhhh
		std::unordered_map<std::string, rawrbox::Vector4f> data = {}; // Other data

		// UTILS ----
		[[nodiscard]] const std::string& getName() const;
		void setName(const std::string& name);

		[[nodiscard]] const std::vector<rawrbox::VertexData>& getVertices() const;
		[[nodiscard]] const std::vector<uint16_t>& getIndices() const;
		[[nodiscard]] const rawrbox::BBOX& getBBOX() const;

		[[nodiscard]] const bool empty() const;

		void setMatrix(const rawrbox::Matrix4x4& matrix);

		[[nodiscard]] const rawrbox::Vector3f& getPos() const;
		void setPos(const rawrbox::Vector3f& pos);

		[[nodiscard]] const rawrbox::Vector4f& getAngle() const;
		void setAngle(const rawrbox::Vector4f& ang);
		void setEulerAngle(const rawrbox::Vector3f& ang);

		[[nodiscard]] const rawrbox::Vector3f& getScale() const;
		void setScale(const rawrbox::Vector3f& scale);

		template <typename B>
		B* getOwner() {
			if (this->owner == nullptr) return nullptr;
			return std::bit_cast<B*>(this->owner);
		}

		[[nodiscard]] const rawrbox::TextureBase* getTexture() const;
		void setTexture(rawrbox::TextureBase* ptr);

		[[nodiscard]] const rawrbox::TextureBase* getNormalTexture() const;
		void setNormalTexture(rawrbox::TextureBase* ptr);

		[[nodiscard]] const rawrbox::TextureBase* getEmissionTexture() const;
		void setEmissionTexture(rawrbox::TextureBase* ptr, float intensity);

		[[nodiscard]] const rawrbox::TextureBase* getOpacityTexture() const;
		void setOpacityTexture(rawrbox::TextureBase* ptr);

		[[nodiscard]] const rawrbox::TextureBase* getSpecularTexture() const;
		void setSpecularTexture(rawrbox::TextureBase* ptr, float shininess);

		void setVertexSnap(float power = 2.F);

		void setDisplacement(float power);

		void setWireframe(bool wireframe);

		void setCulling(uint64_t culling);

		void setDepthTest(uint64_t depthTest);

		void setBlend(uint64_t blend);

		void setColor(const rawrbox::Color& color);
		void setSpecularColor(const rawrbox::Color& color);
		void setEmissionColor(const rawrbox::Color& color);

		void addData(const std::string& id, rawrbox::Vector4f data);
		[[nodiscard]] const rawrbox::Vector4f& getData(const std::string& id) const;
		[[nodiscard]] bool hasData(const std::string& id) const;

		[[nodiscard]] rawrbox::Skeleton* getSkeleton() const;

		void clear();

		void merge(const rawrbox::Mesh& other);
		void setOptimizable(bool status);
		[[nodiscard]] bool canOptimize(const rawrbox::Mesh& other) const;
	};
} // namespace rawrbox
