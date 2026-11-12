#pragma once

#include <rawrbox/render/render_config.hpp>
#include <rawrbox/render/static.hpp>
#include <rawrbox/render/threading/render_thread_pool.hpp>
#include <rawrbox/utils/logger.hpp>

#include <MapHelper.hpp>

#include <Buffer.h>

#include <array>
#include <concepts>
#include <cstdint>
#include <limits>
#include <type_traits>

namespace rawrbox {
	class UniformCache {
	protected:
		static constexpr uint64_t NO_GENERATION = std::numeric_limits<uint64_t>::max();

		template <typename T>
		struct Slot {
			uint64_t generation = NO_GENERATION;
			T value = {};
		};

		static std::array<uint64_t, RB_RENDER_MAX_CONTEXTS> _generation;

	public:
		template <typename T, typename Key = T>
			requires(std::equality_comparable<T> && std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>)
		static bool map(Diligent::IDeviceContext& context, Diligent::IBuffer* buffer, const T& value) {
			if (buffer == nullptr) RAWRBOX_CRITICAL("Invalid uniform buffer");

			static std::array<Slot<T>, RB_RENDER_MAX_CONTEXTS> slots = {};

			const uint32_t index = rawrbox::RenderThreadPool::contextIndex();
			const uint64_t generation = UniformCache::_generation[index];

			auto& cache = slots[index];
			if (cache.generation == generation && cache.value == value) return false;

			// MAP SETUP ---------------
			{
				Diligent::MapHelper<T> mapped(&context, buffer, Diligent::MAP_WRITE, Diligent::MAP_FLAG_DISCARD);
				if (mapped == nullptr) RAWRBOX_CRITICAL("Failed to map uniform buffer!");

				std::memcpy(static_cast<T*>(mapped), &value, sizeof(T));
			}
			// ------------------------

			cache.value = value;
			cache.generation = generation;
			return true;
		}

		template <typename T, typename Key = T>
			requires(std::equality_comparable<T> && std::is_trivially_copyable_v<T> && std::is_default_constructible_v<T>)
		static bool map(Diligent::IBuffer* buffer, const T& value) {
			return UniformCache::map<T, Key>(*rawrbox::RENDERER->context(), buffer, value);
		}

		static void reset();
		static void clear();
	};
} // namespace rawrbox