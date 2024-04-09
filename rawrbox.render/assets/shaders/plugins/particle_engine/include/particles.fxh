#ifndef INCLUDED_PARTICLES
	#define INCLUDED_PARTICLES
	#include "math.fxh"

	struct Particle {
		float3 position;    // 12 bytes
		float lifeTime;     // 4 bytes

		float3 velocity;    // 12 bytes
		float atlasIndex;   // 4 bytes

		float3 rotation;    // 12 bytes

		float2 size;        // 8 bytes
		float4 color;       // 16 bytes
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
