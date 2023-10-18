
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

		// /std::array<uint32_t, 2> normal = {0, 0}; // normal, tangent
		// /
		// /std::array<uint8_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		// /std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		static uint32_t getSizeOf(bool normals = false, bool bones = false) {
			uint32_t offset = sizeof(rawrbox::Vector3f) + sizeof(rawrbox::Vector4f) + sizeof(rawrbox::Colorf);
			// if (normals) offset += sizeof(normal);
			// if (bones) offset += sizeof(bone_indices) + sizeof(bone_weights);

			return offset;
		};

		static std::vector<Diligent::LayoutElement> vLayout(bool normals = false, bool bones = false) {
			std::vector<Diligent::LayoutElement> v = {
			    // Attribute 0 - Position
			    Diligent::LayoutElement{0, 0, 3, Diligent::VT_FLOAT32, false},
			    // Attribute 1 - UV
			    Diligent::LayoutElement{1, 0, 4, Diligent::VT_FLOAT32, false},
			    // Attribute 2 - Color
			    Diligent::LayoutElement{2, 0, 4, Diligent::VT_FLOAT32, false}};

			/*uint32_t indx = 2;
			uint32_t offset = 0;
			if (normals) {
				// Attribute 3 - NORMALS
				v.emplace_back(++indx, 0, 2, Diligent::VT_UINT32, true);
			} else {
				offset = sizeof(normal);
			}

			if (bones) {
				// Attribute 3 ? 4 - BONE-INDICES
				v.emplace_back(++indx, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_UINT32, false, offset);
				// Attribute 4 ? 5 - BONE-WEIGHTS
				v.emplace_back(++indx, 0, rawrbox::MAX_BONES_PER_VERTEX, Diligent::VT_FLOAT32, false, offset + sizeof(bone_indices));
			} else {
				offset = sizeof(bone_indices) + sizeof(bone_weights);
			}*/

			return v;
		}
	};

	struct ModelVertexData {
		rawrbox::VertexData data = {};

		// BLEND HELPERS ---
		// rawrbox::Vector3f ori_pos = {};
		// uint32_t ori_norm = {};
		// -----------------

		ModelVertexData() = default;
		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			data.position = _pos;
			data.color = cl;
			data.uv = _uv;
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			data.position = _pos;
			data.color = cl;
			data.uv = _uv;
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {},
		    const std::array<uint32_t, 2>& _normal = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			data.position = _pos;
			data.color = cl;
			data.uv = _uv;
		}

		explicit ModelVertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {},
		    const std::array<uint32_t, 2>& _normal = {}, const rawrbox::Color cl = rawrbox::Colors::White()) {
			data.position = _pos;
			data.color = cl;
			data.uv = _uv;
		}

		// BLEND SHAPE UTILS ---
		void reset() {
			// this->data.position = this->ori_pos;
			// this->data.normal[0] = this->ori_norm;
		}
		// ---------------------

		// GPU Picker ---
		void setId(uint32_t _id) {
			// if (id > 0x00FFFFFF) throw std::runtime_error("[RawrBox-Mesh] Invalid id");
			// this->id = 0xFF000000 | _id; // Alpha is not supported;
		}
		// ---------------------

		// Atlas ---
		void setAtlasId(uint32_t _id) {
			this->data.uv.z = static_cast<float>(_id);
		}
		// ---------------------

		// BONES UTILS -----
		/*int index = 0;
		void addBoneData(uint8_t boneId, float weight) {
			if (index < rawrbox::MAX_BONES_PER_VERTEX) {
				this->data.bone_indices[index] = boneId;
				this->data.bone_weights[index] = weight;

				index++;
			} else {
				// find the bone with the smallest weight
				int minIndex = 0;
				float minWeight = this->data.bone_weights[0];
				for (int i = 1; i < rawrbox::MAX_BONES_PER_VERTEX; i++) {
					if (this->data.bone_weights[i] < minWeight) {
						minIndex = i;
						minWeight = this->data.bone_weights[i];
					}
				}

				// replace with new bone if the new bone has greater weight
				if (weight > minWeight) {
					this->data.bone_indices[minIndex] = boneId;
					this->data.bone_weights[minIndex] = weight;
				}
			}
		}*/
	};
} // namespace rawrbox
