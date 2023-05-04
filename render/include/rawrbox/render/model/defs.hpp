

#include <rawrbox/math/color.hpp>
#include <rawrbox/math/vector2.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/render/static.hpp>

#include <bgfx/bgfx.h>

#include <array>

namespace rawrBox {

	struct VertexData {
		std::array<float, 3> position = {0, 0, 0};
		std::array<float, 2> uv = {0, 0};
		uint32_t abgr = 0xFFFFFFFF;

		VertexData() = default;
		VertexData(const rawrBox::Vector3f& _pos,
		    float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : position({_pos.x, _pos.y, _pos.z}), uv({_u, _v}), abgr(rawrBox::Color::toHEX(cl)) {}

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

	struct VertexLitData : public VertexData {
		std::array<uint32_t, 2> normal = {0, 0}; // normal, tangent

		VertexLitData() = default;
		VertexLitData(const rawrBox::Vector3f& _pos, uint32_t _normal, uint32_t _tangent,
		    float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : VertexData(_pos, _u, _v, cl), normal({_normal, _tangent}) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Uint8, true, true)

			    .end();
			return l;
		}
	};

	struct VertexSkinnedUnlitData : public VertexData {
		std::array<uint8_t, rawrBox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrBox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexSkinnedUnlitData() = default;
		VertexSkinnedUnlitData(const rawrBox::Vector3f& _pos, float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : VertexData(_pos, _u, _v, cl) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Indices, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, false, true)
			    .add(bgfx::Attrib::Weight, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float)

			    .skip(sizeof(int))

			    .end();
			return l;
		}

		// BONES UTILS -----
		int index = 0;
		void addBoneData(uint8_t boneId, float weight) {
			if (index < rawrBox::MAX_BONES_PER_VERTEX) {
				this->bone_indices[index] = boneId;
				this->bone_weights[index] = weight;

				index++;
			} else {
				// find the bone with the smallest weight
				int minIndex = 0;
				float minWeight = this->bone_weights[0];
				for (int i = 1; i < rawrBox::MAX_BONES_PER_VERTEX; i++) {
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
		std::array<uint8_t, rawrBox::MAX_BONES_PER_VERTEX> bone_indices = {};
		std::array<float, rawrBox::MAX_BONES_PER_VERTEX> bone_weights = {};

		VertexSkinnedLitData() = default;
		VertexSkinnedLitData(const rawrBox::Vector3f& _pos, uint32_t _normal, uint32_t _tangent,
		    float _u, float _v, const rawrBox::Color cl = rawrBox::Colors::White) : VertexLitData(_pos, _normal, _tangent, _u, _v, cl) {}

		static bgfx::VertexLayout vLayout() {
			static bgfx::VertexLayout l;
			l.begin()
			    .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			    .add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Uint8, true, true)
			    .add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Uint8, true, true)

			    .add(bgfx::Attrib::Indices, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Uint8, false, true)
			    .add(bgfx::Attrib::Weight, rawrBox::MAX_BONES_PER_VERTEX, bgfx::AttribType::Float)

			    .skip(sizeof(int))
			    .end();
			return l;
		}

		// BONES UTILS -----
		int index = 0;
		void addBoneData(uint8_t boneId, float weight) {
			if (index < rawrBox::MAX_BONES_PER_VERTEX) {
				this->bone_indices[index] = boneId;
				this->bone_weights[index] = weight;

				index++;
			} else {
				// find the bone with the smallest weight
				int minIndex = 0;
				float minWeight = this->bone_weights[0];
				for (int i = 1; i < rawrBox::MAX_BONES_PER_VERTEX; i++) {
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
	// ---

} // namespace rawrBox
