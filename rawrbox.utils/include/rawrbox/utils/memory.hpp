#pragma once

#include <rawrbox/utils/string.hpp>

#include <fmt/format.h>

#include <memory>
#include <stdexcept>

namespace rawrbox {
	template <typename M, typename T>
	static inline std::weak_ptr<M> cast(const std::weak_ptr<T>& ref) {
		if (ref.expired()) throw std::runtime_error("[RawrBox-Memory] Invalid ref, failed to cast!");

		std::weak_ptr<M> ptr = std::dynamic_pointer_cast<M>(ref.lock());
		if (ptr.expired()) throw std::runtime_error(fmt::format("[RawrBox-Memory] Failed to cast {} to {}", rawrbox::StrUtils::replace(typeid(T).name(), "class rawrbox::", ""), rawrbox::StrUtils::replace(typeid(M).name(), "class rawrbox::", "")));

		return ptr;
	}
} // namespace rawrbox
