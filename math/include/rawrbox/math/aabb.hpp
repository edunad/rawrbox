#pragma once

#include <rawrbox/math/vector2.hpp>

namespace rawrBox {
	template <class NumberType>
	class AABB_t {
	private:
	public:
		Vector2_t<NumberType> pos;
		Vector2_t<NumberType> size;

		AABB_t() = default;

		AABB_t(NumberType x_, NumberType y_, NumberType w_, NumberType h_) : pos(x_, y_), size(w_, h_) {}
		AABB_t(const Vector2_t<NumberType>& pos_, const Vector2_t<NumberType>& size_) : pos(pos_), size(size_) {}

		NumberType surfaceArea() const {
			return size.x * size.y;
		}

		NumberType top() const {
			return pos.y;
		}

		NumberType right() const {
			return pos.x + size.x;
		}

		NumberType left() const {
			return pos.x;
		}

		NumberType bottom() const {
			return pos.y + size.y;
		}

		bool contains(const Vector2_t<NumberType>& pos_) const {
			return pos_.x >= pos.x &&          // left
			       pos_.y >= pos.y &&          // top
			       pos_.x <= pos.x + size.x && // right
			       pos_.y <= pos.y + size.y;   // bottom
		}

		AABB_t<NumberType> mask(const AABB_t<NumberType>& other) const {
			auto masked = *this;

			if (masked.right() < other.left()) masked = {other.left(), other.top(), 0.0F, 0.0F};
			if (masked.bottom() < other.top()) masked = {other.left(), other.top(), 0.0F, 0.0F};
			if (masked.top() > other.bottom()) masked = {other.left(), other.top(), 0.0F, 0.0F};
			if (masked.left() > other.right()) masked = {other.left(), other.top(), 0.0F, 0.0F};

			if (masked.left() < other.left()) {
				masked.size.x -= other.left() - std::abs(masked.left());
				masked.pos.x = other.left();
			}
			if (masked.top() < other.top()) {
				masked.size.y -= other.top() - std::abs(masked.top());
				masked.pos.y = other.top();
			}

			if (masked.right() > other.right()) masked.size.x = other.right() - masked.left();
			if (masked.bottom() > other.bottom()) masked.size.y = other.bottom() - masked.top();

			return masked;
		}
	};

	using AABBd = AABB_t<double>;
	using AABBf = AABB_t<float>;
	using AABBi = AABB_t<int>;
	using AABB = AABBf;
} // namespace rawrBox
