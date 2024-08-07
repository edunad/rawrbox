
#include <rawrbox/physics/manager.hpp>

namespace rawrbox {
	// Private
	std::unique_ptr<JPH::JobSystemThreadPool> PHYSICS::_threadPool = nullptr;
	std::unique_ptr<JPH::Factory> PHYSICS::_factory = nullptr;

	const std::unique_ptr<rawrbox::BPLayerInterface> PHYSICS::_bpLayerInterface = std::make_unique<rawrbox::BPLayerInterface>();
	const std::unique_ptr<rawrbox::BPLayerFilter> PHYSICS::_bpLayerFilter = std::make_unique<rawrbox::BPLayerFilter>();
	const std::unique_ptr<rawrbox::LayerFilter> PHYSICS::_layerFilter = std::make_unique<rawrbox::LayerFilter>();

	std::unique_ptr<rawrbox::BodyActivationListener> PHYSICS::_bodyListener = nullptr;
	std::unique_ptr<rawrbox::ContactListener> PHYSICS::_contactListener = nullptr;
	// ---

	// Public
	std::unique_ptr<JPH::TempAllocatorImpl> PHYSICS::allocator = nullptr;
	std::unique_ptr<JPH::PhysicsSystem> PHYSICS::physicsSystem = nullptr;

	JPH::Ref<JPH::Shape> PHYSICS::rayBoxShape = nullptr;

	rawrbox::Event<const JPH::BodyID&, uint64_t> PHYSICS::onBodyAwake;
	rawrbox::Event<const JPH::BodyID&, uint64_t> PHYSICS::onBodySleep;

	std::function<bool(rawrbox::PHYS_LAYERS, rawrbox::PHYS_LAYERS)> PHYSICS::shouldCollide = nullptr;

	std::function<JPH::ValidateResult(const JPH::Body&, const JPH::Body&, JPH::RVec3Arg, const JPH::CollideShapeResult&)> PHYSICS::onContactValidate = nullptr;

	rawrbox::Event<const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&> PHYSICS::onContactAdded;
	rawrbox::Event<const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&> PHYSICS::onContactPersisted;
	rawrbox::Event<const JPH::SubShapeIDPair&> PHYSICS::onContactRemoved;
	// ---

	// Default settings ---
	int PHYSICS::steps = 1;
	bool PHYSICS::simulate = true;
	// ---

	void PHYSICS::init(uint32_t mbAlloc, uint32_t maxBodies, uint32_t maxBodyMutexes, uint32_t maxBodyPairs, uint32_t maxContactConstraints, uint32_t maxThreads) {
		// Register allocation hook
		JPH::RegisterDefaultAllocator();

		_factory = std::make_unique<JPH::Factory>();
		JPH::Factory::sInstance = _factory.get(); // Initialize singleton factory

		// Register all Jolt physics types
		JPH::RegisterTypes();

		// Initialize allocator
		allocator = std::make_unique<JPH::TempAllocatorImpl>(mbAlloc * 1024 * 1024); // MB

		// Initialize pool
		if (maxThreads == 0) maxThreads = std::thread::hardware_concurrency() - 1;
		_threadPool = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, maxThreads);

		_bodyListener = std::make_unique<rawrbox::BodyActivationListener>();
		_contactListener = std::make_unique<rawrbox::ContactListener>();

		// Initialize physics system
		physicsSystem = std::make_unique<JPH::PhysicsSystem>();
		physicsSystem->Init(maxBodies, maxBodyMutexes, maxBodyPairs, maxContactConstraints, *_bpLayerInterface, *_bpLayerFilter, *_layerFilter);
		physicsSystem->SetBodyActivationListener(_bodyListener.get());
		physicsSystem->SetContactListener(_contactListener.get());

		// Create raycast shape --
		// NOLINTBEGIN(*)
		rayBoxShape = new JPH::BoxShape(JPH::Vec3(1, 1, 1) / 2.F);
		// NOLINTEND(*)
		//  ------------------------
	}

	void PHYSICS::shutdown() {
		rayBoxShape->Release();

		JPH::UnregisterTypes();

		_factory.reset();
		JPH::Factory::sInstance = nullptr;

		allocator.reset();
		_threadPool.reset();

		_bodyListener.reset();
		_contactListener.reset();

		physicsSystem.reset();
	}

	void PHYSICS::clear() {
		JPH::BodyInterface& bodyInterface = physicsSystem->GetBodyInterface();

		JPH::BodyIDVector bodies = {};
		physicsSystem->GetBodies(bodies);

		for (JPH::BodyID id : bodies) {
			bodyInterface.DeactivateBody(id);
			bodyInterface.RemoveBody(id);
			bodyInterface.DestroyBody(id);
		}
	}

	void PHYSICS::tick() {
		if (allocator == nullptr || _threadPool == nullptr || physicsSystem == nullptr || !simulate) return;
		physicsSystem->Update(rawrbox::FIXED_DELTA_TIME, steps, allocator.get(), _threadPool.get());
	}

	void PHYSICS::optimize() {
		if (physicsSystem == nullptr) return;
		physicsSystem->OptimizeBroadPhase();
	}
} // namespace rawrbox
