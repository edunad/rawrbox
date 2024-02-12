#pragma once

#include <rawrbox/math/vector3.hpp>

namespace rawrbox {
	template <class NumberType>
	struct BBOX_t {
	protected:
		using BBOXType = BBOX_t<NumberType>;

	public:
		rawrbox::Vector3_t<NumberType> _min = {};
		rawrbox::Vector3_t<NumberType> _max = {};
		rawrbox::Vector3_t<NumberType> _size = {};

		BBOX_t() = default;
		BBOX_t(const Vector3_t<NumberType>& min, const Vector3_t<NumberType>& max, const Vector3_t<NumberType>& size) : _min(min), _max(max), _size(size) {}

		[[nodiscard]] bool isEmpty() const {
			return this->_size == 0;
		}

		[[nodiscard]] const rawrbox::Vector3_t<NumberType>& size() const {
			return this->_size;
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
