
#include <rawrbox/physics/manager.hpp>
#include <rawrbox/utils/thread_utils.hpp>

#include <fmt/format.h>
#include <tracy/Tracy.hpp>

#include <cstring>
#include <new>

#ifdef JPH_EXTERNAL_PROFILE
// JOLT PROFILER ---
JPH::ExternalProfileMeasurement::ExternalProfileMeasurement(const char* inName, JPH::uint32 /*inColor*/) {
	new (this->mUserData) tracy::ScopedZone(TracyLine, TracyFile, sizeof(TracyFile) - 1, TracyFunction, sizeof(TracyFunction) - 1, inName, std::strlen(inName), 0U);
}

JPH::ExternalProfileMeasurement::~ExternalProfileMeasurement() {
	std::launder(reinterpret_cast<tracy::ScopedZone*>(this->mUserData))->~ScopedZone();
}
// -----------------
#endif

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

		PHYSICS::_factory = std::make_unique<JPH::Factory>();
		JPH::Factory::sInstance = PHYSICS::_factory.get(); // Initialize singleton factory

		// Register all Jolt physics types
		JPH::RegisterTypes();

		// Initialize allocator
		PHYSICS::allocator = std::make_unique<JPH::TempAllocatorImpl>(mbAlloc * 1024 * 1024); // MB

		// Initialize pool
		if (maxThreads == 0) maxThreads = std::thread::hardware_concurrency() - 1;
		PHYSICS::_threadPool = std::make_unique<JPH::JobSystemThreadPool>();
		PHYSICS::_threadPool->SetThreadInitFunction([](int index) { rawrbox::ThreadUtils::setName(fmt::format("rawrbox:physics:{}", index)); });
		PHYSICS::_threadPool->Init(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, maxThreads);

		PHYSICS::_bodyListener = std::make_unique<rawrbox::BodyActivationListener>();
		PHYSICS::_contactListener = std::make_unique<rawrbox::ContactListener>();

		// Initialize physics system
		PHYSICS::physicsSystem = std::make_unique<JPH::PhysicsSystem>();
		PHYSICS::physicsSystem->Init(maxBodies, maxBodyMutexes, maxBodyPairs, maxContactConstraints, *PHYSICS::_bpLayerInterface, *PHYSICS::_bpLayerFilter, *PHYSICS::_layerFilter);
		PHYSICS::physicsSystem->SetBodyActivationListener(PHYSICS::_bodyListener.get());
		PHYSICS::physicsSystem->SetContactListener(PHYSICS::_contactListener.get());

		// Create raycast shape --
		// NOLINTBEGIN(*)
		PHYSICS::rayBoxShape = new JPH::BoxShape(JPH::Vec3(1, 1, 1) / 2.F);
		// NOLINTEND(*)
		//  ------------------------
	}

	void PHYSICS::shutdown() {
		if (PHYSICS::physicsSystem == nullptr) return;

		PHYSICS::rayBoxShape->Release();
		PHYSICS::rayBoxShape = nullptr;

		JPH::UnregisterTypes();

		PHYSICS::_factory.reset();
		JPH::Factory::sInstance = nullptr;

		PHYSICS::allocator.reset();
		PHYSICS::_threadPool.reset();

		PHYSICS::_bodyListener.reset();
		PHYSICS::_contactListener.reset();

		PHYSICS::physicsSystem.reset();
	}

	void PHYSICS::clear() {
		JPH::BodyInterface& bodyInterface = PHYSICS::physicsSystem->GetBodyInterface();

		JPH::BodyIDVector bodies = {};
		PHYSICS::physicsSystem->GetBodies(bodies);

		for (JPH::BodyID id : bodies) {
			bodyInterface.DeactivateBody(id);
			bodyInterface.RemoveBody(id);
			bodyInterface.DestroyBody(id);
		}
	}

	void PHYSICS::tick() {
		if (PHYSICS::allocator == nullptr || PHYSICS::_threadPool == nullptr || PHYSICS::physicsSystem == nullptr || !PHYSICS::simulate) return;
		ZoneScopedN("PHYSICS::tick");
		PHYSICS::physicsSystem->Update(rawrbox::FIXED_DELTA_TIME, PHYSICS::steps, PHYSICS::allocator.get(), PHYSICS::_threadPool.get());
	}

	void PHYSICS::optimize() {
		if (PHYSICS::physicsSystem == nullptr) return;
		PHYSICS::physicsSystem->OptimizeBroadPhase();
	}
} // namespace rawrbox
