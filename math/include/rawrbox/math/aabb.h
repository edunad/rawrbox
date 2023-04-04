#pragma once

#include <rawrbox/math/vector2.h>

namespace rawrbox::math {
	template <class NumberType>
	class AABB_t {
	private:

	public:
		Vector2_t<NumberType> pos;
		Vector2_t<NumberType> size;

		AABB_t() = default;
		~AABB_t() = default;

		AABB_t(NumberType x_, NumberType y_ = 0, NumberType w_ = 0, NumberType h_ = 0) : pos(x_, y_), size(w_, h_) {}
		AABB_t(const Vector2_t<NumberType>& pos_, const Vector2_t<NumberType>& size_) : pos(pos_), size(size_) {}

		AABB_t<NumberType> clamp(const AABB_t<NumberType>& boundries) {
			AABB_t<NumberType> r = *this;

			if (r.pos.x < boundries.pos.x) {
				r.size.x += boundries.pos.x - r.pos.x;
				r.pos.x = boundries.pos.x;
			}

			if (r.pos.y < boundries.pos.y) {
				r.size.y += boundries.pos.y - r.pos.y;
				r.pos.y = boundries.pos.y;
			}

			float wdiff = (r.pos.x + r.size.x) - (boundries.pos.x + boundries.size.x);
			if (wdiff > 0) r.size.x -= wdiff;

			float hdiff = (r.pos.y + r.size.y) - (boundries.pos.y + boundries.size.y);
			if (hdiff > 0) r.size.y -= hdiff;

			return r;
		}

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
			return pos_.x >= pos.x && // left
				pos_.y >= pos.y && // top
				pos_.x <= pos.x + size.x && // right
				pos_.y <= pos.y + size.y; // bottom
		}

		AABB_t<NumberType> mask(const AABB_t<NumberType>& other) const {
			auto masked = *this;

			if (masked.right() < other.left()) masked =  {other.left(), other.top(), 0.0f, 0.0f};
			if (masked.bottom() < other.top()) masked =  {other.left(), other.top(), 0.0f, 0.0f};
			if (masked.top() > other.bottom()) masked =  {other.left(), other.top(), 0.0f, 0.0f};
			if (masked.left() > other.right()) masked =  {other.left(), other.top(), 0.0f, 0.0f};

			if (masked.left() < other.left()) { masked.size.x -= other.left() - std::abs(masked.left()); masked.pos.x = other.left(); }
			if (masked.top() < other.top())   { masked.size.y -= other.top()  - std::abs(masked.top());  masked.pos.y = other.top();  }

			if (masked.right() > other.right())   masked.size.x = other.right()  - masked.left();
			if (masked.bottom() > other.bottom()) masked.size.y = other.bottom() - masked.top();

			return masked;
		}
	};

	using AABBd = AABB_t<double>;
	using AABBf = AABB_t<float>;
	using AABBi = AABB_t<int>;
	using AABB = AABBf;
}
