#pragma once

#include <rawrbox/engine/static.hpp>
#include <rawrbox/math/vector3.hpp>
#include <rawrbox/utils/event.hpp>

// Jolt includes
#include <Jolt/Jolt.h>
//--
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
//---

#include <magic_enum.hpp>

#include <fmt/format.h>

#include <memory>

using namespace JPH::literals; // If you want your code to compile using single or double precision write 0.0_r to get a Real value that compiles to double or float depending if JPH_DOUBLE_PRECISION is set or not.

namespace rawrbox {
	enum class PHYS_LAYERS : JPH::ObjectLayer {
		STATIC = 0,
		DYNAMIC = 1
	};

	class LayerFilter : public JPH::ObjectLayerPairFilter {
	public:
		[[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override {
			switch (static_cast<rawrbox::PHYS_LAYERS>(inObject1)) {
				case rawrbox::PHYS_LAYERS::STATIC:
					return static_cast<PHYS_LAYERS>(inObject2) == rawrbox::PHYS_LAYERS::DYNAMIC;
				case rawrbox::PHYS_LAYERS::DYNAMIC:
					return true;
				default:
					return false;
			}
		}
	};

	class BPLayerInterface final : public JPH::BroadPhaseLayerInterface {
	protected:
		std::vector<JPH::BroadPhaseLayer> _objectToBroadPhase = {};

	public:
		BPLayerInterface() {
			// Create a mapping table from object to broad phase layer
			constexpr auto phys = magic_enum::enum_values<rawrbox::PHYS_LAYERS>();
			for (PHYS_LAYERS p : phys)
				this->_objectToBroadPhase.emplace_back(static_cast<uint8_t>(p));
		}

		[[nodiscard]] uint32_t GetNumBroadPhaseLayers() const override {
			return static_cast<uint32_t>(this->_objectToBroadPhase.size());
		}

		[[nodiscard]] JPH::BroadPhaseLayer GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override {
			if (inLayer >= this->_objectToBroadPhase.size()) throw std::runtime_error(fmt::format("[RawrBox-Physics] Missing physics layer {}!", inLayer));
			return this->_objectToBroadPhase[inLayer];
		}

#if defined(JPH_EXTERNAL_PROFILE) || defined(JPH_PROFILE_ENABLED)
		[[nodiscard]] const char* GetBroadPhaseLayerName(JPH::BroadPhaseLayer inLayer) const override {
			auto a = static_cast<uint8_t>(inLayer);
			const auto* name = magic_enum::enum_name(static_cast<rawrbox::PHYS_LAYERS>(a)).data();
			return name;
		}
#endif
	};

	class BodyActivationListener;
	class ContactListener;
	class BPLayerFilter;

	class PHYSICS {
	protected:
		static std::unique_ptr<JPH::JobSystemThreadPool> _threadPool;
		static std::unique_ptr<JPH::Factory> _factory;

		static const std::unique_ptr<rawrbox::BPLayerInterface> _bpLayerInterface;
		static const std::unique_ptr<rawrbox::BPLayerFilter> _bpLayerFilter;
		static const std::unique_ptr<rawrbox::LayerFilter> _layerFilter;

		static std::unique_ptr<rawrbox::BodyActivationListener> _bodyListener;
		static std::unique_ptr<rawrbox::ContactListener> _contactListener;

	public:
		// VARS ----
		static std::unique_ptr<JPH::TempAllocatorImpl> allocator;
		static std::unique_ptr<JPH::PhysicsSystem> physicsSystem;

		static JPH::Ref<JPH::Shape> rayBoxShape;

		static int steps;
		static bool simulate;

		static rawrbox::Event<const JPH::BodyID&, uint64_t> onBodyAwake;
		static rawrbox::Event<const JPH::BodyID&, uint64_t> onBodySleep;
		static std::function<bool(rawrbox::PHYS_LAYERS, rawrbox::PHYS_LAYERS)> shouldCollide;

		static std::function<JPH::ValidateResult(const JPH::Body&, const JPH::Body&, JPH::RVec3Arg, const JPH::CollideShapeResult&)> onContactValidate;

		static rawrbox::Event<const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&> onContactAdded;
		static rawrbox::Event<const JPH::Body&, const JPH::Body&, const JPH::ContactManifold&, JPH::ContactSettings&> onContactPersisted;
		static rawrbox::Event<const JPH::SubShapeIDPair&> onContactRemoved;
		// ----

		static void init(uint32_t mbAlloc = 20, uint32_t maxBodies = 2048, uint32_t maxBodyMutexes = 2048, uint32_t maxBodyPairs = 2048, uint32_t maxContactConstraints = 2048, uint32_t maxThreads = 0);
		static void shutdown();

		static void clear();
		static void tick();     // Should be tick based update
		static void optimize(); // Call only when a lot of bodies are added at a single time
	};

	class BPLayerFilter : public JPH::ObjectVsBroadPhaseLayerFilter {
	public:
		[[nodiscard]] bool ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override {
			auto layer1 = static_cast<rawrbox::PHYS_LAYERS>(inLayer1);
			auto layer2 = static_cast<rawrbox::PHYS_LAYERS>(static_cast<uint8_t>(inLayer2));

			if (PHYSICS::shouldCollide == nullptr) {
				switch (layer1) {
					case rawrbox::PHYS_LAYERS::STATIC:
						return layer2 == rawrbox::PHYS_LAYERS::DYNAMIC;
					case rawrbox::PHYS_LAYERS::DYNAMIC:
						return true;
					default:
						return false;
				}
			} else {
				return PHYSICS::shouldCollide(layer1, layer2);
			}
		}
	};

	class BodyActivationListener : public JPH::BodyActivationListener {
	public:
		void OnBodyActivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override {
			PHYSICS::onBodyAwake(inBodyID, inBodyUserData);
		}

		void OnBodyDeactivated(const JPH::BodyID& inBodyID, uint64_t inBodyUserData) override {
			PHYSICS::onBodySleep(inBodyID, inBodyUserData);
		}
	};

	class ContactListener : public JPH::ContactListener {
	public:
		// See: ContactListener
		JPH::ValidateResult OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override {
			if (rawrbox::PHYSICS::onContactValidate == nullptr) return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
			return rawrbox::PHYSICS::onContactValidate(inBody1, inBody2, inBaseOffset, inCollisionResult);
		}

		void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
			rawrbox::PHYSICS::onContactAdded(inBody1, inBody2, inManifold, ioSettings);
		}

		void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override {
			rawrbox::PHYSICS::onContactPersisted(inBody1, inBody2, inManifold, ioSettings);
		}

		void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override {
			rawrbox::PHYSICS::onContactRemoved(inSubShapePair);
		}
	};
} // namespace rawrbox
