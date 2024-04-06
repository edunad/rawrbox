#ifndef INCLUDED_PARTICLES
	#define INCLUDED_PARTICLES
	#include "math.fxh"

	struct Particle {
		float3 position;
		float lifeTime;

		float3 velocity;
		float atlasIndex;

		float2 size;
		float2 rotation;

		float4 color;
	};

	#if defined(WRITE_PARTICLES)
		RWStructuredBuffer<Particle> Particles; // Read-Write
		#define PARTICLES
	#elif defined(READ_PARTICLES)
		StructuredBuffer<Particle> Particles; // Read-only
		#define PARTICLES
	#endif

	#ifdef PARTICLES
	Particle GetParticle(uint index) {
	    return Particles[NonUniformResourceIndex(index)];
	}
	#endif
#endif
