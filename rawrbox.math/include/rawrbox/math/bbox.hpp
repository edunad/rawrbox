#pragma once

#include <rawrbox/math/vector3.hpp>

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

		[[nodiscard]] bool isEmpty() const {
			return this->size == 0;
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

			this->size = this->min.abs() + this->max.abs();
		}

		bool operator==(const BBOXType& other) const { return this->size == other.size; }
		bool operator!=(const BBOXType& other) const { return !operator==(other); }

		BBOXType operator*(NumberType other) const { return BBOXType(this->min * other, this->max * other, this->size * other); }
		BBOXType operator*(const rawrbox::Vector3_t<NumberType>& other) const { return BBOXType(this->min * other, this->max * other, this->size * other); }
	};

	using BBOXd = BBOX_t<double>;
	using BBOXf = BBOX_t<float>;
	using BBOXi = BBOX_t<int>;
	using BBOX = BBOXf;
} // namespace rawrbox
