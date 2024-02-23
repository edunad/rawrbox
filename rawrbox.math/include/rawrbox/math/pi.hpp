#pragma once

namespace rawrbox {
	template <typename T>
	constexpr T pi = static_cast<T>(3.14159265358979323846L);

	template <typename T>
	constexpr T piHalf = pi<T> / 2.F;

	template <typename T>
	constexpr T TWO = static_cast<T>(2.F);

	template <typename T>
	constexpr T ONE = static_cast<T>(1.F);

	template <typename T>
	constexpr T ZERO = static_cast<T>(0.F);

	template <typename T>
	constexpr T HALFONE = static_cast<T>(0.5F);
} // namespace rawrbox
