#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/utils/logger.hpp>

#include <DeviceContext.h>
#include <DeviceObject.h>

#include <vector>

namespace rawrbox {
	class BarrierUtils {
	protected:
		static std::unique_ptr<rawrbox::Logger> _logger;

	public:
		static void barrier(const std::vector<Diligent::StateTransitionDesc>& resources);
		// ----------------
	};
} // namespace rawrbox
