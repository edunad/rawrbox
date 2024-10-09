#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/utils/logger.hpp>

#include <DeviceContext.h>
#include <DeviceObject.h>

#include <unordered_map>

namespace rawrbox {
	class BarrierUtils {
	protected:
		// BARRIER -------------
		static std::unordered_map<Diligent::IDeviceObject*, Diligent::StateTransitionDesc> _barrierCache;
		//  -------------------------

		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static void barrier(const std::vector<Diligent::StateTransitionDesc>& resources);
		static void clearBarrierCache();
		// ----------------
	};
} // namespace rawrbox
