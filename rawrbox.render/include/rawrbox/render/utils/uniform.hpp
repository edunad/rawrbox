#pragma once

#include <RefCntAutoPtr.hpp>
#include <Buffer.h>

namespace rawrbox {
	class UniformUtils {
	public:
		[[nodiscard]] static Diligent::RefCntAutoPtr<Diligent::IBuffer> create(const char* name, const void* data, uint64_t size);
		static void update(Diligent::IBuffer* buffer, const void* data, uint64_t size);
	};
} // namespace rawrbox