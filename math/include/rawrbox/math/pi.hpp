#pragma once

namespace rawrbox {
	template <typename T>
	constexpr T pi = static_cast<T>(3.14159265358979323846L);

	template <typename T>
	constexpr T piHalf = pi<T> / 2.F;
} // namespace rawrbox
