
#pragma once

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/static.hpp>

#include <bgfx/bgfx.h>

#include <array>

namespace rawrbox {
	struct VertexData {
		rawrbox::Vector3f position = {};
		rawrbox::Vector4f uv = {};
		uint32_t abgr = 0xFFFFFFFF;
		uint32_t id = 0x00000000;

		std::array<uint32_t, 2> normal = {0, 0}; // normal, tangent

		std::array<uint8_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		// BLEND HELPERS ---
		rawrbox::Vector3f _ori_pos = {};
		uint32_t _ori_norm = {};
		// -----------------

		VertexData() = default;
		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), abgr(cl.pack()), _ori_pos(position) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), abgr(cl.pack()), _ori_pos(position) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {},
		    const std::array<uint32_t, 2>& _normal = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), abgr(cl.pack()), normal(_normal), _ori_pos(position), _ori_norm(normal[0]) {}

		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {},
		    const std::array<uint32_t, 2>& _normal = {}, const rawrbox::Color cl = rawrbox::Colors::White()) : position(_pos), uv(_uv), abgr(cl.pack()), normal(_normal), _ori_pos(position), _ori_norm(normal[0]) {}

		static bgfx::VertexLayout vLayout(bool normals = false, bool bones = false) {
			bgfx::VertexLayout l;

			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Color1, 4, bgfx::AttribType::Uint8, true, true);

			if (normals) {
				l.add(bgfx::Attrib::Normal, 4, bgfx::AttribType::Uint8, true, true)
				    .add(bgfx::Attrib::Tangent, 4, bgfx::AttribType::Uint8, true, true);
			} else {
				l.skip(sizeof(normal));
			}

			if (bones) {
				l.add(bgfx::Attrib::Indices, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, true, true)
				    .add(bgfx::Attrib::Weight, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float);
			} else {
				l.skip(sizeof(bone_indices));
				l.skip(sizeof(bone_weights));
			}

			l.skip(sizeof(int)).skip(sizeof(_ori_pos)).skip(sizeof(_ori_norm)); // skip helpers
			l.end();

			return l;
		}

		// BLEND SHAPE UTILS ---
		void reset() {
			this->position = this->_ori_pos;
			this->normal[0] = this->_ori_norm;
		}
		// ---------------------

		// GPU Picker ---
		void setId(uint32_t _id) {
			this->id = 0xFF000000 | _id; // Alpha is not supported;
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
		// ----
	};
} // namespace rawrbox
