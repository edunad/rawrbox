#pragma once

#include <rawrbox/engine/static.hpp>

#include <DeviceContext.h>
#include <DeviceObject.h>

#include <unordered_map>

namespace rawrbox {
	class BarrierUtils {
	protected:
		// BARRIER -------------
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::RESOURCE_STATE> _barrierCache;
		//  -------------------------

	public:
		static void barrier(const std::vector<Diligent::StateTransitionDesc>& resources);
		static void clearBarrierCache();
		// ----------------
	};
} // namespace rawrbox
