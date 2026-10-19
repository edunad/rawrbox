
#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/utils/pack.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/static.hpp>

#include <InputLayout.h>

#include <array>
#include <cstdint>

namespace rawrbox {
	// BONES ---
	using BoneIndices = std::array<uint8_t, RB_MAX_BONES_PER_VERTEX>;
	using BoneWeights = std::array<uint16_t, RB_MAX_BONES_PER_VERTEX>; // UNORM16, 65535 = 1.0, see PackUtils::packBoneWeights
	// ---------

	struct VertexData {
		rawrbox::Vector3f position = {};
		float slice = 0.F; // Texture array slice

		constexpr VertexData() = default;
		constexpr VertexData(const rawrbox::Vector3f& _pos) : position(_pos) {}

		void setPos(const rawrbox::Vector3f& _pos) { this->position = _pos; }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position (xyz) + Slice (w)
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(1, 1, 1, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Instance index
			}

			return v;
		}
	};

	struct VertexUVData : public rawrbox::VertexData {
		rawrbox::Vector2f uv = {};

		constexpr VertexUVData() = default;
		constexpr VertexUVData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}) : rawrbox::VertexData(_pos), uv(_uv) {}

		// Texture array ---
		void setSlice(uint32_t _id) { this->slice = static_cast<float>(_id); }
		[[nodiscard]] uint32_t getSlice() const { return static_cast<uint32_t>(this->slice); }
		// ---------------------

		void setUV(const rawrbox::Vector2f& _uv) { this->uv = _uv; }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position (xyz) + Slice (w)
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(2, 1, 1, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Instance index
			}

			return v;
		}
	};

	// Supports light ---
	struct VertexNormData : public rawrbox::VertexUVData {
		uint32_t normal = 0x00000000;
		uint32_t tangent = 0x00000000;

		constexpr VertexNormData() = default;
		VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, float tangentSign = 1.F) : rawrbox::VertexUVData(_pos, _uv), normal(rawrbox::PackUtils::packOCTNormal(norm.x, norm.y, norm.z)), tangent(rawrbox::PackUtils::packOCTTangent(tang.x, tang.y, tang.z, tangentSign)) {}
		constexpr VertexNormData(const rawrbox::Vector3f& _pos, const rawrbox::Vector2f& _uv = {}, uint32_t _norm = 0x00000000, uint32_t _tang = 0x00000000) : rawrbox::VertexUVData(_pos, _uv), normal(_norm), tangent(_tang) {}

		void setNormal(const rawrbox::Vector3f& norm) { this->normal = rawrbox::PackUtils::packOCTNormal(norm.x, norm.y, norm.z); }
		void setTangent(const rawrbox::Vector3f& tang, float sign = 1.F) { this->tangent = rawrbox::PackUtils::packOCTTangent(tang.x, tang.y, tang.z, sign); }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position (xyz) + Slice (w)
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Normal
			    Diligent::LayoutElement{2, 0, 2, Diligent::VT_UINT16, true},
			    // Attribute 3 - Tangent
			    Diligent::LayoutElement{3, 0, 2, Diligent::VT_UINT16, false},
			};

			if (instanced) {
				v.emplace_back(4, 1, 1, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Instance index
			}

			return v;
		}
	};

	// Supports bones ---
	struct VertexBoneData : public rawrbox::VertexUVData {
		rawrbox::BoneIndices bone_indices = {};
		rawrbox::BoneWeights bone_weights = {}; // UNORM16

		void setBones(const std::array<uint32_t, RB_MAX_BONES_PER_VERTEX>& indices, const std::array<float, RB_MAX_BONES_PER_VERTEX>& weights) {
			this->bone_indices = rawrbox::PackUtils::packBoneIndices(indices);
			this->bone_weights = rawrbox::PackUtils::packBoneWeights(weights);
		}

		constexpr VertexBoneData() = default;
		constexpr VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}) : rawrbox::VertexUVData(_pos, _uv) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position (xyz) + Slice (w)
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - BONE-INDICES
			    Diligent::LayoutElement{2, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT8, false},
			    // Attribute 3 - BONE-WEIGHTS
			    Diligent::LayoutElement{3, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT16, true}};

			if (instanced) {
				v.emplace_back(4, 1, 1, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Instance index
			}

			return v;
		}
	};

	// Supports light && bones ---
	struct VertexNormBoneData : public rawrbox::VertexNormData {
		rawrbox::BoneIndices bone_indices = {};
		rawrbox::BoneWeights bone_weights = {}; // UNORM16

		void setBones(const std::array<uint32_t, RB_MAX_BONES_PER_VERTEX>& indices, const std::array<float, RB_MAX_BONES_PER_VERTEX>& weights) {
			this->bone_indices = rawrbox::PackUtils::packBoneIndices(indices);
			this->bone_weights = rawrbox::PackUtils::packBoneWeights(weights);
		}

		constexpr VertexNormBoneData() = default;
		VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, float tangentSign = 1.F) : rawrbox::VertexNormData(_pos, _uv, norm, tang, tangentSign) {}

		constexpr VertexNormBoneData(const rawrbox::Vector3f& _pos, const rawrbox::Vector2f& _uv = {}, uint32_t norm = 0x00000000, uint32_t tang = 0x00000000) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position (xyz) + Slice (w)
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 2, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Normal
			    Diligent::LayoutElement{2, 0, 2, Diligent::VT_UINT16, true},
			    // Attribute 3 - Tangent
			    Diligent::LayoutElement{3, 0, 2, Diligent::VT_UINT16, false},
			    // Attribute 4 - BONE-INDICES
			    Diligent::LayoutElement{4, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT8, false},
			    // Attribute 5 - BONE-WEIGHTS
			    Diligent::LayoutElement{5, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT16, true}};

			if (instanced) {
				v.emplace_back(6, 1, 1, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Instance index
			}

			return v;
		}
	};

	// UTILS ---
	template <typename T>
	concept supportsBones = requires(T t) { t.bone_indices; };

	template <typename T>
	concept supportsNormals = requires(T t) { t.normal; };

	template <typename T>
	concept supportsUVs = requires(T t) { t.uv; };
	// ---
} // namespace rawrbox
