#pragma once

#include <rawrbox/math/vector2.hpp>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
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

		[[nodiscard]] bool contains(const AABB_t<NumberType>& b) const {
			return (this->pos.x <= (b.pos.x + b.size.x) && (this->pos.x + this->size.x) >= b.pos.x &&
				this->pos.y <= (b.pos.y + b.size.y) && (this->pos.y + this->size.y) >= b.pos.y);
		}

		[[nodiscard]] static AABB_t<NumberType> intersects(const AABB_t<NumberType>& a, const AABB_t<NumberType>& b) {
			AABB_t<NumberType> result;

			if (a.contains(b)) {
				result.pos.x = std::max(a.pos.x, b.pos.x);
				result.size.x = std::min(a.pos.x + a.size.x, b.pos.x + b.size.x) - result.pos.x;
				result.pos.y = std::max(a.pos.y, b.pos.y);
				result.size.y = std::min(a.pos.y + a.size.y, b.pos.y + b.size.y) - result.pos.y;
			}

			return result;
		}

		[[nodiscard]] AABB_t<NumberType> mask(const AABB_t<NumberType>& other) const {
			auto masked = *this;

			if (masked.right() < other.left()) masked = {other.left(), other.top(), 0, 0};
			if (masked.bottom() < other.top()) masked = {other.left(), other.top(), 0, 0};
			if (masked.top() > other.bottom()) masked = {other.left(), other.top(), 0, 0};
			if (masked.left() > other.right()) masked = {other.left(), other.top(), 0, 0};

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

		template <class ReturnType>
		AABB_t<ReturnType> cast() const {
			return {pos.template cast<ReturnType>(), size.template cast<ReturnType>()};
		}

		AABBType operator*(NumberType other) const { return AABBType(this->pos * other, this->size * other); }

		bool operator==(const AABBType& other) const { return this->pos == other.pos && this->size == other.size; }
		bool operator!=(const AABBType& other) const { return !operator==(other); }
	};

	using AABBd = AABB_t<double>;
	using AABBf = AABB_t<float>;
	using AABBi = AABB_t<int>;
	using AABB = AABBf;
} // namespace rawrbox
