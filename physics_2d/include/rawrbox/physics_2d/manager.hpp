#pragma once
#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/utils/event.hpp>

#include <fmt/format.h>
#include <muli/muli.h>

#include <memory>

namespace rawrbox {
	class PHYSICS_2D {
	protected:
	public:
		static std::unique_ptr<muli::World> physWorld;

		// VARS ----
		static int velocitySteps;
		static int positionSteps;
		//  ----

		static void init(const muli::Vec2& gravity = {0.0F, -10.0F});
		static void shutdown();

		static void tick(); // Should be tick based update
	};
} // namespace rawrbox
