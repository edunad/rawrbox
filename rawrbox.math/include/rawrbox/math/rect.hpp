#pragma once

#include <rawrbox/math/vector2.hpp>

#include <algorithm>
#include <limits>

namespace rawrbox {
	template <class NumberType>
		requires(std::is_integral_v<NumberType> || std::is_floating_point_v<NumberType>)
	struct Rect_t {
	protected:
		using RectType = Rect_t<NumberType>;

	public:
		rawrbox::Vector2_t<NumberType> min = {};
		rawrbox::Vector2_t<NumberType> max = {};

		Rect_t() = default;

		constexpr Rect_t(const rawrbox::Vector2_t<NumberType>& _min, const rawrbox::Vector2_t<NumberType>& _max) : min(_min), max(_max) {}
		constexpr Rect_t(NumberType minX, NumberType minY, NumberType maxX, NumberType maxY) : min(minX, minY), max(maxX, maxY) {}

		[[nodiscard]] static constexpr RectType empty() {
			constexpr NumberType l = std::numeric_limits<NumberType>::lowest();
			constexpr NumberType h = std::numeric_limits<NumberType>::max();

			return {h, h, l, l};
		}

		[[nodiscard]] bool isEmpty() const {
			return this->min.x > this->max.x || this->min.y > this->max.y;
		}

		[[nodiscard]] bool overlaps(const RectType& other) const {
			if (this->isEmpty() || other.isEmpty()) return false;
			return this->min.x <= other.max.x && other.min.x <= this->max.x && this->min.y <= other.max.y && other.min.y <= this->max.y;
		}

		void combine(const RectType& other) {
			if (other.isEmpty()) return;

			if (this->isEmpty()) {
				*this = other;
				return;
			}

			this->min = {std::min(this->min.x, other.min.x), std::min(this->min.y, other.min.y)};
			this->max = {std::max(this->max.x, other.max.x), std::max(this->max.y, other.max.y)};
		}

		[[nodiscard]] RectType clamp(const RectType& bounds) const {
			return {std::max(this->min.x, bounds.min.x), std::max(this->min.y, bounds.min.y), std::min(this->max.x, bounds.max.x), std::min(this->max.y, bounds.max.y)};
		}

		// 8 bits per edge (0 - 255), empty packs as min 255 / max 0 ---
		[[nodiscard]] uint32_t pack() const
			requires std::is_integral_v<NumberType>
		{
			if (this->isEmpty()) return 0x0000FFFFU;
			return static_cast<uint32_t>(this->min.x) | (static_cast<uint32_t>(this->min.y) << 8U) | (static_cast<uint32_t>(this->max.x) << 16U) | (static_cast<uint32_t>(this->max.y) << 24U);
		}
		// -----------

		[[nodiscard]] RectType expand(NumberType amount) const {
			if (this->isEmpty()) return *this;
			return {this->min.x - amount, this->min.y - amount, this->max.x + amount, this->max.y + amount};
		}

		bool operator==(const RectType& other) const { return this->min == other.min && this->max == other.max; }
		bool operator!=(const RectType& other) const { return !operator==(other); }
	};

	using Rectf = Rect_t<float>;
	using Recti = Rect_t<int>;
	using Rectu = Rect_t<uint32_t>;
} // namespace rawrbox