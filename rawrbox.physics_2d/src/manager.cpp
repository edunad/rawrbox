
#include <rawrbox/physics_2d/manager.hpp>

#include <tracy/Tracy.hpp>

namespace rawrbox {
	// PUBLIC ----
	std::unique_ptr<muli::WorldSettings> PHYSICS_2D::physSettings = std::make_unique<muli::WorldSettings>();
	std::unique_ptr<muli::World> PHYSICS_2D::physWorld = nullptr;

	// EVENTS -----
	rawrbox::Event<const muli::ContactManifold&> PHYSICS_2D::onContact = {};
	// ----
	// -----

	void PHYSICS_2D::init() {
		PHYSICS_2D::physWorld = std::make_unique<muli::World>(*PHYSICS_2D::physSettings); // Setup world
	}

	void PHYSICS_2D::shutdown() {
		PHYSICS_2D::physSettings.reset();
		PHYSICS_2D::physWorld.reset();
	}

	void PHYSICS_2D::tick() {
		if (PHYSICS_2D::physWorld == nullptr) return;
		ZoneScopedN("PHYSICS_2D::tick");

		PHYSICS_2D::physWorld->Step(rawrbox::FIXED_DELTA_TIME);
		PHYSICS_2D::checkContacts();
	}

	void PHYSICS_2D::checkContacts() {
		if (PHYSICS_2D::physWorld == nullptr || PHYSICS_2D::onContact.empty()) return;

		const muli::Contact* c = PHYSICS_2D::physWorld->GetContacts();
		while (c != nullptr) {
			if (!c->IsTouching()) {
				c = c->GetNext();
				continue;
			}

			PHYSICS_2D::onContact(c->GetContactManifold());
			c = c->GetNext();
		}
	}
} // namespace rawrbox
