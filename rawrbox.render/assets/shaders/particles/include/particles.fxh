#ifndef INCLUDED_PARTICLES
	#define INCLUDED_PARTICLES
    #include "math.fxh"

	struct Particle {
		float4 position;
        float4 velocity;
        float4 color;
        float4 data; // X = Lifetime, Y = TextureID, Z = ???, W = ???
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
