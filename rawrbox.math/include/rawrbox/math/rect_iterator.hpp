#pragma once

#include <rawrbox/math/vector2.hpp>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType>)
	class RectIterator_t {
	protected:
		rawrbox::Vector2_t<NumberType> _point = {};
		NumberType _minX = 0;
		NumberType _maxX = 0;

	public:
		RectIterator_t() = default;
		constexpr RectIterator_t(const rawrbox::Vector2_t<NumberType>& point, NumberType minX, NumberType maxX) : _point(point), _minX(minX), _maxX(maxX) {}

		[[nodiscard]] const rawrbox::Vector2_t<NumberType>& operator*() const { return this->_point; }

		RectIterator_t& operator++() {
			if (this->_point.x == this->_maxX) {
				this->_point.x = this->_minX;
				++this->_point.y;
			} else {
				++this->_point.x;
			}

			return *this;
		}

		bool operator==(const RectIterator_t& other) const { return this->_point == other._point; }
	};

	template <class NumberType>
		requires(std::is_integral_v<NumberType>)
	struct RectRange_t {
		rawrbox::RectIterator_t<NumberType> first = {};
		rawrbox::RectIterator_t<NumberType> last = {};

		[[nodiscard]] rawrbox::RectIterator_t<NumberType> begin() const { return this->first; }
		[[nodiscard]] rawrbox::RectIterator_t<NumberType> end() const { return this->last; }
	};
} // namespace rawrbox
