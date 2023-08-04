
#include <rawrbox/physics_2d/manager.hpp>

namespace rawrbox {
	// PUBLIC ----
	std::unique_ptr<muli::World> PHYSICS_2D::physWorld = nullptr;
	// -----

	// Default settings ---
	int PHYSICS_2D::velocitySteps = 1;
	int PHYSICS_2D::positionSteps = 1;
	// ---

	void PHYSICS_2D::init(const muli::Vec2& gravity) {
		muli::WorldSettings settings;

		settings.velocity_iterations = velocitySteps;
		settings.position_iterations = positionSteps;
		settings.gravity = gravity;

		physWorld = std::make_unique<muli::World>(settings);
	}

	void PHYSICS_2D::shutdown() {
		physWorld.reset();
	}

	void PHYSICS_2D::tick() {
		if (physWorld == nullptr) return;
		physWorld->Step(rawrbox::FIXED_DELTA_TIME);
	}
} // namespace rawrbox
