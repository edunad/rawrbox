#ifndef INCLUDED_PARTICLES_UNIFORMS
    #define INCLUDED_PARTICLES_UNIFORMS

    struct EmitterConstantsStruct {
        float3 position;
        float time;

        float3 velocityMin;
        float lifeMin;

        float3 velocityMax;
        float lifeMax;

        float3 rotationMin;
        float spawnRate;

        float3 rotationMax;
        float gravity;

        float4 color[4];
        float4 size;

        // ------------
        uint billboard;
        uint atlasMin;
        uint atlasMax;
        uint textureID;
        // ------------
    };

    ConstantBuffer<EmitterConstantsStruct> EmitterConstants;
#endif
