
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
		float w = 1.F; // padding

		rawrbox::Vector4f16 uv = {};

		VertexData() = default;
		VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}) : position(_pos), uv(_uv.pack()) {}
		VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f16& _uv = {}) : position(_pos), uv(_uv) {}

		void setUV(const rawrbox::Vector4f& _uv) { this->uv = _uv.pack(); }
		void setPos(const rawrbox::Vector3f& _pos) { this->position = _pos; }

		// Texture array ---
		void setSlice(uint32_t _id) { this->uv.z = static_cast<int16_t>(_id); }
		[[nodiscard]] uint32_t getSlice() const { return static_cast<uint16_t>(this->uv.z); }
		// ---------------------

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT16, false}};

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
	struct VertexNormData : public rawrbox::VertexData {
		uint32_t normal = 0x00000000;
		uint32_t tangent = 0x00000000;

		VertexNormData() = default;
		VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}) : rawrbox::VertexData(_pos, _uv), normal(rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z)), tangent(rawrbox::PackUtils::packNormal(tang.x, tang.y, tang.z)) {}
		VertexNormData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _uv = {}, uint32_t _norm = 0x00000000, uint32_t _tang = 0x00000000) : rawrbox::VertexData(_pos, _uv), normal(_norm), tangent(_tang) {}
		VertexNormData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f16& _uv = {}, uint32_t _norm = 0x00000000, uint32_t _tang = 0x00000000) : rawrbox::VertexData(_pos, _uv), normal(_norm), tangent(_tang) {}

		void setNormal(const rawrbox::Vector3f& norm) { normal = rawrbox::PackUtils::packNormal(norm.x, norm.y, norm.z); }
		void setTangent(const rawrbox::Vector3f& tang) { tangent = rawrbox::PackUtils::packNormal(tang.x, tang.y, tang.z); }

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT16, false},
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
	struct VertexBoneData : public rawrbox::VertexData {
		std::array<uint32_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<uint16_t, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexBoneData() = default;
		VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}) : rawrbox::VertexData(_pos, _uv) {}
		VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f16& _uv = {}) : rawrbox::VertexData(_pos, _uv) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT16, false},
			    // Attribute 2 - BONE-INDICES
			    Diligent::LayoutElement{2, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 3 - BONE-WEIGHTS
			    Diligent::LayoutElement{3, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT16, false}};

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
		std::array<uint32_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<uint16_t, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexNormBoneData() = default;
		VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}

		VertexNormBoneData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f& _uv = {}, uint32_t norm = 0x00000000, uint32_t tang = 0x00000000) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}
		VertexNormBoneData(const rawrbox::Vector3f& _pos, const rawrbox::Vector4f16& _uv = {}, uint32_t norm = 0x00000000, uint32_t tang = 0x00000000) : rawrbox::VertexNormData(_pos, _uv, norm, tang) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT16, false},
			    // Attribute 2 - Normal
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - Tangent
			    Diligent::LayoutElement{3, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 4 - BONE-INDICES
			    Diligent::LayoutElement{4, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 5 - BONE-WEIGHTS
			    Diligent::LayoutElement{5, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT16, false}};

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
	// ---
} // namespace rawrbox
