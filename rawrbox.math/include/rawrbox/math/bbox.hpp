#pragma once

#include <rawrbox/math/matrix4x4.hpp>
#include <rawrbox/math/vector3.hpp>

#include <algorithm>
#include <limits>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	struct BBOX_t {
	protected:
		using BBOXType = BBOX_t<NumberType>;

	public:
		rawrbox::Vector3_t<NumberType> min = {};
		rawrbox::Vector3_t<NumberType> max = {};
		rawrbox::Vector3_t<NumberType> size = {};

		BBOX_t() = default;
		constexpr BBOX_t(const Vector3_t<NumberType>& _min, const Vector3_t<NumberType>& _max, const Vector3_t<NumberType>& _size) : min(_min), max(_max), size(_size) {}

		[[nodiscard]] static BBOXType infinite() {
			constexpr NumberType low = std::numeric_limits<NumberType>::lowest();
			constexpr NumberType high = std::numeric_limits<NumberType>::max();

			return {{low, low, low}, {high, high, high}, {high, high, high}};
		}

		[[nodiscard]] bool isEmpty() const {
			return this->size == 0;
		}

		[[nodiscard]] bool isInfinite() const {
			return this->min.x == std::numeric_limits<NumberType>::lowest();
		}

		void expand(const rawrbox::Vector3_t<NumberType>& pos) {
			if (pos.x < this->min.x) this->min.x = pos.x;
			if (pos.y < this->min.y) this->min.y = pos.y;
			if (pos.z < this->min.z) this->min.z = pos.z;

			if (pos.x > this->max.x) this->max.x = pos.x;
			if (pos.y > this->max.y) this->max.y = pos.y;
			if (pos.z > this->max.z) this->max.z = pos.z;

			this->size = this->max - this->min;
		}

		bool contains(const rawrbox::Vector3_t<NumberType>& pos) {
			return pos.x >= this->min.x && pos.x <= this->max.x && pos.y >= this->min.y && pos.y <= this->max.y && pos.z >= this->min.z && pos.z <= this->max.z;
		}

		void combine(const BBOXType& b) {
			this->min = {std::min(this->min.x, b.min.x), std::min(this->min.y, b.min.y), std::min(this->min.z, b.min.z)};
			this->max = {std::max(this->max.x, b.max.x), std::max(this->max.y, b.max.y), std::max(this->max.z, b.max.z)};

			this->size = this->max - this->min;
		}

		// Axis aligned bounds ---
		[[nodiscard]] BBOXType transform(const rawrbox::Matrix4x4& matrix) const
			requires std::is_same_v<NumberType, float>
		{
			if (this->isInfinite()) return *this;

			const auto first = matrix.mulVec(this->min);
			BBOXType result = {first, first, {}};

			for (uint32_t corner = 1; corner < 8; corner++) {
				result.expand(matrix.mulVec(rawrbox::Vector3f{
				    (corner & 1U) != 0 ? this->max.x : this->min.x,
				    (corner & 2U) != 0 ? this->max.y : this->min.y,
				    (corner & 4U) != 0 ? this->max.z : this->min.z}));
			}

			return result;
		}
		// -----------

		// FRUSTUM ---
		[[nodiscard]] bool inFrustum(const rawrbox::Matrix4x4& viewProj) const
			requires std::is_same_v<NumberType, float>
		{
			if (this->isInfinite()) return true;

			uint32_t outside = 0x3FU;

			for (uint32_t corner = 0; corner < 8; corner++) {
				const auto clip = viewProj.mulVec(rawrbox::Vector4f{
				    (corner & 1U) != 0 ? this->max.x : this->min.x,
				    (corner & 2U) != 0 ? this->max.y : this->min.y,
				    (corner & 4U) != 0 ? this->max.z : this->min.z,
				    1.F});

				uint32_t planes = 0;

				if (clip.x < -clip.w) planes |= 1U;
				if (clip.x > clip.w) planes |= 2U;
				if (clip.y < -clip.w) planes |= 4U;
				if (clip.y > clip.w) planes |= 8U;
				if (clip.z < -clip.w) planes |= 16U;
				if (clip.z > clip.w) planes |= 32U;

				outside &= planes;
				if (outside == 0) return true;
			}

			return false;
		}
		// -----------

		bool operator==(const BBOXType& other) const { return this->min == other.min && this->max == other.max; }
		bool operator!=(const BBOXType& other) const { return !operator==(other); }

		BBOXType operator*(NumberType other) const { return BBOXType(this->min * other, this->max * other, this->size * other); }
		BBOXType operator*(const rawrbox::Vector3_t<NumberType>& other) const { return BBOXType(this->min * other, this->max * other, this->size * other); }
	};

	using BBOXd = BBOX_t<double>;
	using BBOXf = BBOX_t<float>;
	using BBOXi = BBOX_t<int>;
	using BBOX = BBOXf;
} // namespace rawrbox