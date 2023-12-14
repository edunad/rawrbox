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
		static void checkContacts();

	public:
		static std::unique_ptr<muli::WorldSettings> physSettings;
		static std::unique_ptr<muli::World> physWorld;

		// EVENTS -----
		static rawrbox::Event<const muli::ContactManifold&> onContact;
		// ----

		static void init();
		static void shutdown();

		static void tick(); // Should be tick based update
	};
} // namespace rawrbox
