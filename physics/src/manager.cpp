
#include <rawrbox/physics/manager.hpp>

namespace rawrbox {
	// Private
	std::unique_ptr<JPH::TempAllocatorImpl> rawrbox::PHYSICS::_allocator = nullptr;
	std::unique_ptr<JPH::JobSystemThreadPool> rawrbox::PHYSICS::_threadPool = nullptr;
	// ---

	// Public
	std::shared_ptr<JPH::PhysicsSystem> rawrbox::PHYSICS::physicsSystem = nullptr;
	// ---

	// Default settings ---
	int rawrbox::PHYSICS::steps = 1;
	int rawrbox::PHYSICS::subSteps = 1;
	// ---
} // namespace rawrbox
