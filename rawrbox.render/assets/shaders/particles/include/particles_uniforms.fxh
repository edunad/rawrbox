#ifndef INCLUDED_PARTICLES_UNIFORMS
    #define INCLUDED_PARTICLES_UNIFORMS

    struct EmitterConstantsStruct {
        float3 position;    // 12 bytes
        float time;         // 4 bytes

        float3 velocityMin; // 12 bytes
        float lifeMin;      // 4 bytes

        float3 velocityMax; // 12 bytes
        float lifeMax;      // 4 bytes

        float3 rotationMin; // 12 bytes
        float spawnRate;    // 4 bytes

        float3 rotationMax; // 12 bytes
        float gravity;      // 4 bytes

        uint4 color;        // 16 bytes
        float4 size;        // 16 bytes

        uint billboard;     // 4 bytes
        uint atlasMin;      // 4 bytes
        uint atlasMax;      // 4 bytes
        uint textureID;     // 4 bytes
    };

    ConstantBuffer<EmitterConstantsStruct> EmitterConstants;
#endif
