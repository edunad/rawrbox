#ifndef INCLUDED_PARTICLES_UNIFORMS
    #define INCLUDED_PARTICLES_UNIFORMS

    struct EmitterConstantsStruct {
        float3 position;
        float lifetime;

        float3 velocity;
        float deltaTime;

        uint4 data;
    };

    ConstantBuffer<EmitterConstantsStruct> EmitterConstants;
    #define TOTAL_PARTICLES EmitterConstants.data.x
#endif
