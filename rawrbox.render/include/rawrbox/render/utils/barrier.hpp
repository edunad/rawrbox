#pragma once

#include <rawrbox/engine/static.hpp>

#include <DeviceContext.h>
#include <DeviceObject.h>

#include <functional>
#include <unordered_map>

namespace rawrbox {

	template <typename T>
		requires(std::derived_from<T, Diligent::IDeviceObject>)
	struct Barrier {
	public:
		T* resource = nullptr;
		Diligent::RESOURCE_STATE state = Diligent::RESOURCE_STATE_UNKNOWN;

		Barrier() = default;
		Barrier(T* _resource, Diligent::RESOURCE_STATE _state) : resource(_resource), state(_state){};
	};

	class BarrierUtils {
	protected:
		// BARRIER -------------
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::StateTransitionDesc> _barrierQueue;
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::RESOURCE_STATE> _barrierCache;
		static std::vector<std::function<void()>> _barrierCallback;
		// -------------------------

	public:
		// BARRIERS -------
		template <typename T>
			requires(std::derived_from<T, Diligent::IDeviceObject>)
		static void barrier(const std::vector<rawrbox::Barrier<T>>& resources, const std::function<void()>& callback = nullptr) {
			for (size_t i = 0; i < resources.size(); i++) {
				auto& barrier = resources[i];
				_barrierQueue[barrier.resource] = Diligent::StateTransitionDesc(barrier.resource, Diligent::RESOURCE_STATE_UNKNOWN, barrier.state, Diligent::STATE_TRANSITION_FLAG_UPDATE_STATE);
			}

			if (callback != nullptr) _barrierCallback.push_back(callback);
			if (std::this_thread::get_id() == rawrbox::RENDER_THREAD_ID) processBarriers();
		}

		static void processBarriers();
		static void clearBarrierCache();
		// ----------------
	};
} // namespace rawrbox
