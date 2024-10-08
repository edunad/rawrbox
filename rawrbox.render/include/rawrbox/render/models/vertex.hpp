
#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/static.hpp>

#include <InputLayout.h>

namespace rawrbox {
	struct VertexData {
		rawrbox::Vector3f position = {};
		float w = 1.F; // Padding

		constexpr VertexData() = default;
		constexpr VertexData(const rawrbox::Vector3f& _pos) : position(_pos) {}
		VertexData(const rawrbox::Vector4f& _pos) : position(_pos.xyz()), w(_pos.w) {}

		void setPos(const rawrbox::Vector3f& _pos) { this->position = _pos; }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(1, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(2, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(3, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(5, 1, 4, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Data
			}

			return v;
		}
	};

	struct VertexUVData : public rawrbox::VertexData {
		rawrbox::Vector4f uv = {};

		constexpr VertexUVData() = default;
		constexpr VertexUVData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}) : rawrbox::VertexData(_pos), uv(_uv) {}

		// Texture array ---
		void setSlice(uint32_t _id) { this->uv.z = static_cast<float>(_id); }
		[[nodiscard]] uint32_t getSlice() const { return static_cast<uint16_t>(this->uv.z); }
		// ---------------------

		void setUV(const rawrbox::Vector4f& _uv) { this->uv = _uv; }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(2, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(3, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(6, 1, 4, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Data
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
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}) : rawrbox::VertexUVData(_pos, _uv), normal(rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z)), tangent(rawrbox::PackUtils::packNormal(tang.x, tang.y, tang.z)) {}
		constexpr VertexNormData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _uv = {}, uint32_t _norm = 0x00000000, uint32_t _tang = 0x00000000) : rawrbox::VertexUVData(_pos, _uv), normal(_norm), tangent(_tang) {}

		void setNormal(const rawrbox::Vector3f& norm) { normal = rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z); }
		void setTangent(const rawrbox::Vector3f& tang) { tangent = rawrbox::PackUtils::packNormal(tang.x, tang.y, tang.z); }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Normal
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - Tangent
			    Diligent::LayoutElement{3, 0, 4, Diligent::VT_UINT8, true},
			};

			if (instanced) {
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(8, 1, 4, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Data
			}

			return v;
		}
	};

	// Supports bones ---
	struct VertexBoneData : public rawrbox::VertexUVData {
		std::array<uint32_t, RB_MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, RB_MAX_BONES_PER_VERTEX> bone_weights = {};

		constexpr VertexBoneData() = default;
		constexpr VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}) : rawrbox::VertexUVData(_pos, _uv) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - BONE-INDICES
			    Diligent::LayoutElement{2, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 3 - BONE-WEIGHTS
			    Diligent::LayoutElement{3, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(8, 1, 4, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Data
			}

			return v;
		}
	};

	// Supports light && bones ---
	struct VertexNormBoneData : public rawrbox::VertexNormData {
		std::array<uint32_t, RB_MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, RB_MAX_BONES_PER_VERTEX> bone_weights = {};

		constexpr VertexNormBoneData() = default;
		VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}

		constexpr VertexNormBoneData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _uv = {}, uint32_t norm = 0x00000000, uint32_t tang = 0x00000000) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Normal
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - Tangent
			    Diligent::LayoutElement{3, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 4 - BONE-INDICES
			    Diligent::LayoutElement{4, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 5 - BONE-WEIGHTS
			    Diligent::LayoutElement{5, 0, RB_MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false}}; // TODO, VT_FLOAT16 for bone-weights?

			if (instanced) {
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(9, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(10, 1, 4, Diligent::VT_UINT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Data
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
