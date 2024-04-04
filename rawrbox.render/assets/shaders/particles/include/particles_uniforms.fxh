#ifndef INCLUDED_PARTICLES_UNIFORMS
    #define INCLUDED_PARTICLES_UNIFORMS

    struct EmitterConstantsStruct {
        float3 position;
        float time;

        float4 velocity[2];

        float4 color[4];
        float4 size;

        // ------------
        float2 life;
        float spawnRate;
        float maxParticles;
        // ------------
    };

    ConstantBuffer<EmitterConstantsStruct> EmitterConstants;
#endif
