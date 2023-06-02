#pragma once
#include <rawrbox/engine/static.hpp>

#include <Jolt/Jolt.h>

// Jolt includes
#include <Jolt/Core/Factory.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#include <memory>
#include <thread>

namespace rawrbox {
	class PHYSICS {
	protected:
		static std::unique_ptr<JPH::TempAllocatorImpl> _allocator;
		static std::unique_ptr<JPH::JobSystemThreadPool> _threadPool;
		static std::unique_ptr<JPH::Factory> _factory;

	public:
		static std::shared_ptr<JPH::PhysicsSystem> physicsSystem;

		static int steps;
		static int subSteps;

		static void init(uint32_t maxBodies = 1024, uint32_t maxBodyMutexes = 0, uint32_t maxBodyPairs = 1024, uint32_t maxContactConstraints = 1024, uint32_t maxThreads = 0) {
			// Register allocation hook
			JPH::RegisterDefaultAllocator();

			_factory = std::make_unique<JPH::Factory>();
			JPH::Factory::sInstance = _factory.get(); // Initialize singleton factory

			// Register all Jolt physics types
			JPH::RegisterTypes();

			// Initialize allocator
			_allocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024); // 10 MB

			// Initialize pool
			if (maxThreads == 0) maxThreads = std::thread::hardware_concurrency() - 1;
			_threadPool = std::make_unique<JPH::JobSystemThreadPool>(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, maxThreads);

			// Initialize physics system
			physicsSystem = std::make_shared<JPH::PhysicsSystem>();
			// physicsSystem->Init(maxBodies, maxBodyMutexes, maxBodyPairs, maxContactConstraints,);
		}

		static void shutdown() {
			JPH::UnregisterTypes();

			_factory.reset();
			JPH::Factory::sInstance = nullptr;

			_allocator.reset();
			_threadPool.reset();

			physicsSystem.reset()
		}

		static void tick(bool optimize = true) {
			if (_factory == nullptr || _allocator == nullptr || _threadPool == nullptr || physicsSystem == nullptr) return;
			if (optimize) physicsSystem->OptimizeBroadPhase();

			physicsSystem->Update(rawrbox::DELTA_TIME, steps, subSteps, _allocator.get(), _threadPool.get());
		}
	};
} // namespace rawrbox
