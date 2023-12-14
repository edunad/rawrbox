
#include <rawrbox/physics_2d/manager.hpp>

namespace rawrbox {
	// PUBLIC ----
	std::unique_ptr<muli::WorldSettings> PHYSICS_2D::physSettings = std::make_unique<muli::WorldSettings>();
	std::unique_ptr<muli::World> PHYSICS_2D::physWorld = nullptr;

	// EVENTS -----
	rawrbox::Event<const muli::ContactManifold&> PHYSICS_2D::onContact = {};
	// ----
	// -----

	void PHYSICS_2D::init() {
		physWorld = std::make_unique<muli::World>(*physSettings); // Setup world
	}

	void PHYSICS_2D::shutdown() {
		physSettings.reset();
		physWorld.reset();
	}

	void PHYSICS_2D::tick() {
		if (physWorld == nullptr) return;

		physWorld->Step(rawrbox::FIXED_DELTA_TIME);
		checkContacts();
	}

	void PHYSICS_2D::checkContacts() {
		if (physWorld == nullptr || onContact.empty()) return;

		const muli::Contact* c = physWorld->GetContacts();
		while (c) {
			if (c->IsTouching() == false) {
				c = c->GetNext();
				continue;
			}

			onContact(c->GetContactManifold());
			c = c->GetNext();
		}
	}
} // namespace rawrbox
