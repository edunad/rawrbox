
#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/static.hpp>

#include <Graphics/GraphicsEngine/interface/InputLayout.h>

#include <array>

namespace rawrbox {

	struct VertexData {
		rawrbox::Vector3f position = {};
		rawrbox::Vector4f uv = {};
		rawrbox::Colorf color = {};

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(3, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(4, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Color
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}

		static uint32_t vLayoutSize() {
			return sizeof(VertexData);
		}
	};

	// Supports light ---
	struct VertexNormData : public virtual VertexData {
		rawrbox::Vector3f normal = {};
		rawrbox::Vector3f tangent = {};

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false},
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

				v.emplace_back(9, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);  // Color
				v.emplace_back(10, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}

		static uint32_t vLayoutSize() {
			return sizeof(VertexNormData);
		}
	};

	// Supports bones ---
	struct VertexBoneData : public virtual VertexData {
		std::array<uint8_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 3 - BONE-INDICES
			    Diligent::LayoutElement{3, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false},
			    // Attribute 4 - BONE-WEIGHTS
			    Diligent::LayoutElement{4, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false}};

			if (instanced) {
				v.emplace_back(5, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 1
				v.emplace_back(6, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 2
				v.emplace_back(7, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 3
				v.emplace_back(8, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Matrix - 4

				v.emplace_back(9, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE);  // Color
				v.emplace_back(10, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}

		static uint32_t vLayoutSize() {
			return sizeof(VertexBoneData);
		}
	};

	// Supports light && bones ---
	struct VertexNormBoneData : public VertexNormData, public VertexBoneData {

		VertexNormBoneData() = default;
		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			this->position = _pos;
			this->uv = _uv;
			this->normal = norm;
			this->tangent = tang;
			this->color = cl;
		}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			this->position = _pos;
			this->uv = _uv;
			this->normal = norm;
			this->tangent = tang;
			this->color = cl;
		}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			this->position = _pos;
			this->uv = _uv;
			this->color = cl;
		}

		explicit VertexNormBoneData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			this->position = _pos;
			this->uv = _uv;
			this->color = cl;
		}

		static std::vector<Diligent::LayoutElement> vLayout(bool instanced = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false},
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

				v.emplace_back(11, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Color
				v.emplace_back(12, 1, 4, Diligent::VT_FLOAT32, false, Diligent::INPUT_ELEMENT_FREQUENCY_PER_INSTANCE); // Extra
			}

			return v;
		}

		static uint32_t vLayoutSize() {
			return sizeof(VertexNormBoneData);
		}
	};

	// Supports light && bones ---
	// With helpers, but not to be passed to buffer
	struct ModelVertexData : public VertexNormBoneData {
		// BLEND HELPERS ---
		rawrbox::Vector3f ori_pos = {};
		rawrbox::Vector3f ori_norm = {};
		// -----------------

		using VertexNormBoneData::vLayout;
		using VertexNormBoneData::vLayoutSize;

		ModelVertexData() = default;
		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormBoneData(_pos, _uv, norm, tang, cl), ori_pos(_pos), ori_norm(norm) {
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Vector3f& norm = {}, const rawrbox::Vector3f& tang = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormBoneData(_pos, _uv, norm, tang, cl), ori_pos(_pos), ori_norm(norm) {
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormBoneData(_pos, _uv, cl), ori_pos(_pos) {
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : rawrbox::VertexNormBoneData(_pos, _uv, cl), ori_pos(_pos) {
		}

		// GPU Picker ---
		void setId(uint32_t _id) {
			// if (id > 0x00FFFFFF) throw std::runtime_error("[RawrBox-Mesh] Invalid id");
			// this->id = 0xFF000000 | _id; // Alpha is not supported;
		}
		// ---------------------

		// Atlas ---
		void setAtlasId(uint32_t _id) {
			this->uv.z = static_cast<float>(_id);
		}
		// ---------------------

		// BONES UTILS -----
		int index = 0;
		void addBoneData(uint8_t boneId, float weight) {
			if (index < rawrbox::MAX_BONES_PER_VERTEX) {
				this->bone_indices[index] = boneId;
				this->bone_weights[index] = weight;

				index++;
			} else {
				// find the bone with the smallest weight
				int minIndex = 0;
				float minWeight = this->bone_weights[0];
				for (int i = 1; i < rawrbox::MAX_BONES_PER_VERTEX; i++) {
					if (this->bone_weights[i] < minWeight) {
						minIndex = i;
						minWeight = this->bone_weights[i];
					}
				}

				// replace with new bone if the new bone has greater weight
				if (weight > minWeight) {
					this->bone_indices[minIndex] = boneId;
					this->bone_weights[minIndex] = weight;
				}
			}
		}
		// ------------------

		// BLEND SHAPE UTILS ---
		void reset() {
			this->position = this->ori_pos;
			this->normal = this->ori_norm;
		}
		// ---------------------
	};
} // namespace rawrbox
