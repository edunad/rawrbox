#pragma once

#include <rawrbox/render/materials/particle.hpp>
#include <rawrbox/render/particles/emitter.hpp>

#include <memory>
#include <vector>

namespace rawrbox {
	class PARTICLES {
	protected:
		// Drawing ----
		std::vector<std::unique_ptr<rawrbox::Emitter>> _emitters = {};
		std::unique_ptr<rawrbox::MaterialParticle> _material = std::make_unique<rawrbox::MaterialParticle>();
		// ---

	public:
		static void init();
		static void shutdown();
	};
} // namespace rawrbox
