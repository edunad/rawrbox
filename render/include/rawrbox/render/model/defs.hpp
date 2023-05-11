

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/math/vector4.hpp>
#include <rawrbox/render/static.hpp>

#include <bgfx/bgfx.h>

#include <array>

namespace rawrbox {

	struct VertexData {
		std::array<float, 3> position = {0, 0, 0};
		std::array<float, 2> uv = {0, 0};
		uint32_t abgr = 0xFFFFFFFF;

		VertexData() = default;
		explicit VertexData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector2f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White) : position(_pos.data()), uv(_uv.data()), abgr(cl.pack()) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			    .end();
			return l;
		}
	};

	struct VertexBlendData { // Used for particles
		std::array<float, 3> position = {0, 0, 0};
		std::array<float, 4> uv = {0, 0, 0, 0};
		uint32_t abgr = 0xFFFFFFFF;

		VertexBlendData() = default;
		explicit VertexBlendData(const rawrbox::Vector3f& _pos,
		    const rawrbox::Vector4f& _uv = {}, const rawrbox::Color cl = rawrbox::Colors::White) : position(_pos.data()), uv(_uv.data()), abgr(cl.pack()) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 4, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)
			    .end();
			return l;
		}
	};

	struct VertexLitData : public VertexData {
		std::array<uint32_t, 3> normal = {0, 0, 0}; // normal, tangent, bitangent

		VertexLitData() = default;
		explicit VertexLitData(const rawrbox::Vector3f& _pos, const rawrbox::Vector2f& _uv = {}, const std::array<uint32_t, 3>& _normals = {}, const rawrbox::Color cl = rawrbox::Colors::White) : VertexData(_pos, _uv, cl), normal(_normals) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Uint8, true, true)

			    .end();
			return l;
		}
	};

	struct VertexSkinnedUnlitData : public VertexData {
		std::array<uint8_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		using VertexData::VertexData;

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Indices, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, false, true)
			    .add(bgfx::Attrib::Weight, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float)

			    .skip(sizeof(int)) // skip index

			    .end();

			return l;
		}

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

	struct VertexSkinnedLitData : public VertexLitData {
		std::array<uint8_t, rawrbox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrbox::MAX_BONES_PER_VERTEX> bone_weights = {};

		using VertexLitData::VertexLitData;

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Bitangent, 3, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Indices, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, false, true)
			    .add(bgfx::Attrib::Weight, rawrbox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float)

			    .skip(sizeof(int))
			    .end();
			return l;
		}

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

	// UTILS ---
	template <typename T>
	concept supportsBones = requires(T t, uint8_t boneId, float weight) {
		{ t.addBoneData(boneId, weight) };
	};

	template <typename T>
	concept supportsNormals = requires(T t) { t.normal; };

	template <typename T>
	concept supportsBlend = requires(T t) { t.uv.size() == 4; };
	// ---

} // namespace rawrbox
