#pragma once

#include <rawrbox/math/vector2.hpp>

namespace rawrbox {
	template <class NumberType>
	class AABB_t {
	protected:
		using AABBType = AABB_t<NumberType>;

	public:
		Vector2_t<NumberType> pos;
		Vector2_t<NumberType> size;

		AABB_t() = default;

		AABB_t(NumberType x_, NumberType y_, NumberType w_, NumberType h_) : pos(x_, y_), size(w_, h_) {}
		AABB_t(const Vector2_t<NumberType>& pos_, const Vector2_t<NumberType>& size_) : pos(pos_), size(size_) {}

		[[nodiscard]] NumberType surfaceArea() const {
			return this->size.x * this->size.y;
		}

		[[nodiscard]] NumberType top() const {
			return this->pos.y;
		}

		[[nodiscard]] NumberType right() const {
			return this->pos.x + this->size.x;
		}

		[[nodiscard]] NumberType left() const {
			return this->pos.x;
		}

		[[nodiscard]] NumberType bottom() const {
			return this->pos.y + this->size.y;
		}

		[[nodiscard]] bool empty() const { return this->pos == 0 && this->size == 0; }
		[[nodiscard]] bool contains(const Vector2_t<NumberType>& pos_) const {
			return pos_.x >= this->pos.x &&                // left
			       pos_.y >= this->pos.y &&                // top
			       pos_.x <= this->pos.x + this->size.x && // right
			       pos_.y <= this->pos.y + this->size.y;   // bottom
		}

		[[nodiscard]] AABB_t<NumberType> mask(const AABB_t<NumberType>& other) const {
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

		bool operator==(const AABBType& other) const { return this->pos == other.pos && this->size == other.size; }
		bool operator!=(const AABBType& other) const { return !operator==(other); }
	};

	using AABBd = AABB_t<double>;
	using AABBf = AABB_t<float>;
	using AABBi = AABB_t<int>;
	using AABB = AABBf;
} // namespace rawrbox
