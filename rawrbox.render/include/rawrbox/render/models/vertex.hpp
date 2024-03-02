
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
		rawrbox::Vector4f uv = {};
		uint32_t color = 0xFFFFFFFF;
		// uint32_t id = 0x00000000;

		VertexData() = default;
		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), color(cl.pack()) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), color(cl.pack()) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const uint32_t cl = 0xFFFFFFFF) : position(_pos), uv(_uv), color(cl) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const uint32_t cl = 0xFFFFFFFF) : position(_pos), uv(_uv), color(cl) {}

		// Atlas ---
		void setAtlasId(uint32_t _id) {
			this->uv.z = static_cast<float>(_id);
		}
		// ---------------------

		// GPU Picker ---
		/*void setId(uint32_t _id) {
			if (id > 0x00FFFFFF) throw rawrbox::Logger::err("RawrBox-Mesh", "Invalid id");
			this->id = 0xFF000000 | _id; // Alpha is not supported;
		}*/
		// ---------------------

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true}};

			if (instanced) {
				v.emplace_back(3, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(7, 1, 4, Diligent::VT_UINT8, true, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);    // Color
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}
	};

	// Supports light ---
	struct VertexNormData : public VertexData {
		rawrbox::Vector3f normal = {};
		rawrbox::Vector3f tangent = {};

		VertexNormData() = default;
		explicit VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexData(_pos, _uv, cl), normal(norm), tangent(tang) {}

		explicit VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexData(_pos, _uv, cl), normal(norm), tangent(tang) {}

		explicit VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexData(_pos, _uv, cl), normal(norm), tangent(tang) {}

		explicit VertexNormData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexData(_pos, _uv, cl), normal(norm), tangent(tang) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - Normal
			    Diligent::LayoutElement{3, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 4 - Tangent
			    Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, false},
			};

			if (instanced) {
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(9, 1, 4, Diligent::VT_UINT8, true, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);     // Color
				v.emplace_back(10, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}
	};

	// Supports bones ---
	struct VertexBoneData : public VertexData {
		std::array<uint32_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexBoneData() = default;
		explicit VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexData(_pos, _uv, cl) {}

		explicit VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexData(_pos, _uv, cl) {}

		explicit VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexData(_pos, _uv, cl) {}

		explicit VertexBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexData(_pos, _uv, cl) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - BONE-INDICES
			    Diligent::LayoutElement{3, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 4 - BONE-WEIGHTS
			    Diligent::LayoutElement{4, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(9, 1, 4, Diligent::VT_UINT8, true, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);     // Color
				v.emplace_back(10, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}
	};

	// Supports light && bones ---
	struct VertexNormBoneData : public VertexNormData {
		std::array<uint32_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexNormBoneData() = default;
		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormData(_pos, _uv, norm, tang, cl) {}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormData(_pos, _uv, norm, tang, cl) {}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexNormData(_pos, _uv, norm, tang, cl) {}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const uint32_t cl = 0xFFFFFFFF) : rawrbox::VertexNormData(_pos, _uv, norm, tang, cl) {}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_UINT8, true},
			    // Attribute 3 - Normal
			    Diligent::LayoutElement{3, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 4 - Tangent
			    Diligent::LayoutElement{4, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 5 - BONE-INDICES
			    Diligent::LayoutElement{5, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 6 - BONE-WEIGHTS
			    Diligent::LayoutElement{6, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);  // Matrix - 1
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);  // Matrix - 2
				v.emplace_back(9, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);  // Matrix - 3
				v.emplace_back(10, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(11, 1, 4, Diligent::VT_UINT8, true, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);    // Color
				v.emplace_back(12, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
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
