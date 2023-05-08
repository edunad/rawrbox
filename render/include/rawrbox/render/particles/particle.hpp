#pragma once

#include <rawrbox/math/vector3.hpp>

#include <bx/math.h>

#include <array>

// Adapted from https://github.com/bkaradzic/bgfx/blob/master/examples/32-particles/particles.cpp
namespace rawrbox {
	struct ParticleSort {
		float dist;
		uint32_t idx;
	};

	struct Particle {
	public:
		bx::Vec3 posStart = {0, 0, 0};
		std::array<bx::Vec3, 2> posEnd = {bx::Vec3(0, 0, 0), bx::Vec3(0, 0, 0)};

		std::array<uint32_t, 5> rgba = {};
		uint32_t texture = 0;

		float life = 0.F;
		float lifeSpan = 0.F;

		float blendStart = 0.F;
		float blendEnd = 0.F;

		float scaleStart = 0.F;
		float scaleEnd = 0.F;

		float rotationStart = 0.F;
		float rotationEnd = 0.F;

		Particle() = default;
	};

} // namespace rawrbox
