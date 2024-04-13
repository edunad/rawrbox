#pragma once

#include <rawrbox/math/vector3.hpp>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	struct BBOX_t {
	protected:
		using BBOXType = BBOX_t<NumberType>;

	public:
		rawrbox::Vector3_t<NumberType> _min = {};
		rawrbox::Vector3_t<NumberType> _max = {};
		rawrbox::Vector3_t<NumberType> _size = {};

		BBOX_t() = default;
		constexpr BBOX_t(const Vector3_t<NumberType>& min, const Vector3_t<NumberType>& max, const Vector3_t<NumberType>& size) : _min(min), _max(max), _size(size) {}

		[[nodiscard]] bool isEmpty() const {
			return this->_size == 0;
		}

		[[nodiscard]] const rawrbox::Vector3_t<NumberType>& size() const {
			return this->_size;
		}

		void expand(const rawrbox::Vector3_t<NumberType>& pos) {
			if (pos.x < _min.x) _min.x = pos.x;
			if (pos.y < _min.y) _min.y = pos.y;
			if (pos.z < _min.z) _min.z = pos.z;

			if (pos.x > _max.x) _max.x = pos.x;
			if (pos.y > _max.y) _max.y = pos.y;
			if (pos.z > _max.z) _max.z = pos.z;

			_size = _max - _min;
		}

		bool contains(const rawrbox::Vector3_t<NumberType>& pos) {
			return pos.x >= _min.x && pos.x <= _max.x && pos.y >= _min.y && pos.y <= _max.y && pos.z >= _min.z && pos.z <= _max.z;
		}

		void combine(const BBOX_t<NumberType>& b) {
			this->_min = {std::min(this->_min.x, b._min.x), std::min(this->_min.y, b._min.y), std::min(this->_min.z, b._min.z)};
			this->_max = {std::max(this->_max.x, b._max.x), std::max(this->_max.y, b._max.y), std::max(this->_max.z, b._max.z)};

			this->_size = _min.abs() + _max.abs();
		}

		bool operator==(const BBOX_t<NumberType>& other) const { return this->_size == other._size; }
		bool operator!=(const BBOX_t<NumberType>& other) const { return !operator==(other); }
	};

	using BBOXd = BBOX_t<double>;
	using BBOXf = BBOX_t<float>;
	using BBOXi = BBOX_t<int>;
	using BBOX = BBOXf;
} // namespace rawrbox
