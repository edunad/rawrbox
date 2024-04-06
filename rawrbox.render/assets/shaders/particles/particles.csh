#include "camera.fxh"
#include "particles_uniforms.fxh"
#include "hash.fxh"
#include "math.fxh"

#define WRITE_PARTICLES
#include "particles.fxh"

float3 CalculateVelocity(uint hash) {
    float3 randomVelocity;
    randomVelocity.x = lerp(EmitterConstants.velocityMin.x, EmitterConstants.velocityMax.x, (float)(hash & 0xFF) * (1.0f / 255.0f));
    randomVelocity.y = lerp(EmitterConstants.velocityMin.y, EmitterConstants.velocityMax.y, (float)((hash >> 8) & 0xFF) * (1.0f / 255.0f));
    randomVelocity.z = lerp(EmitterConstants.velocityMin.z, EmitterConstants.velocityMax.z, (float)((hash >> 16) & 0xFF) * (1.0f / 255.0f));
    return randomVelocity;
}

float2 CalculateRotation(uint hash) {
    float2 randomRotation;
    randomRotation.x = lerp(EmitterConstants.rotationMin.x, EmitterConstants.rotationMax.x, (float)(hash & 0xFF) * (1.0f / 255.0f));
    randomRotation.y = lerp(EmitterConstants.rotationMin.y, EmitterConstants.rotationMax.y, (float)((hash >> 8) & 0xFF) * (1.0f / 255.0f));
    return randomRotation;
}

float2 CalculateSize(uint hash) {
    float2 randomSize;
    randomSize.x = lerp(EmitterConstants.size.x, EmitterConstants.size.z, (float)(hash & 0xFFFF) * (1.0f / 65535.0f));
    randomSize.y = lerp(EmitterConstants.size.y, EmitterConstants.size.w, (float)((hash >> 16) & 0xFFFF) * (1.0f / 65535.0f));
    return randomSize;
}

uint CalculateAtlas(uint hash) {
    return (uint)lerp(EmitterConstants.atlasMin, EmitterConstants.atlasMax, (float)(hash & 0xFFFF) * (1.0f / 65535.0f));
}

float CalculateLife(uint hash) {
    return lerp(EmitterConstants.lifeMin, EmitterConstants.lifeMax, (float)(hash & 0xFFFF) * (1.0f / 65535.0f));
}

float4 ParticleColor(float lifetimeRatio) {
    const float transitionPoints[3] = {0.33, 0.66, 0.99};
    lifetimeRatio = saturate(1.0 - lifetimeRatio);

    // Determine color based on lifetimeRatio
    if (lifetimeRatio < transitionPoints[0]) {
        float factor = lifetimeRatio / transitionPoints[0];
        return lerp(EmitterConstants.color[0], EmitterConstants.color[1], factor);
    } else if (lifetimeRatio < transitionPoints[1]) {
        float factor = (lifetimeRatio - transitionPoints[0]) / (transitionPoints[1] - transitionPoints[0]);
        return lerp(EmitterConstants.color[1], EmitterConstants.color[2], factor);
    } else if (lifetimeRatio < transitionPoints[2]) {
        float factor = (lifetimeRatio - transitionPoints[1]) / (transitionPoints[2] - transitionPoints[1]);
        return lerp(EmitterConstants.color[2], EmitterConstants.color[3], factor);
    } else {
        return EmitterConstants.color[3];
    }
}
// Use groupshared memory to reduce bandwidth
groupshared Particle localParticles[256];

[numthreads(256, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex) {
    uint particleIndex = dispatchThreadID.x;
    if (particleIndex >= EmitterConstants.maxParticles) return;

    // Load particle data into groupshared memory
    localParticles[groupIndex] = GetParticle(particleIndex);
    GroupMemoryBarrierWithGroupSync();

    Particle particle = localParticles[groupIndex];
    uint hash = pcg(EmitterConstants.time + particleIndex * 1009);

    // Calculate the spawn interval based on the spawn rate
    float spawnInterval = 1.0f / EmitterConstants.spawnRate;
    float particleSpawnTime = particleIndex * spawnInterval;

    if (particle.lifeTime <= 0.0f && EmitterConstants.time >= particleSpawnTime) {
        // Spawn / reset a particle
        particle.position = EmitterConstants.position;
        particle.color = EmitterConstants.color[0];

        particle.velocity = CalculateVelocity(hash);
        particle.size = CalculateSize(hash);
        particle.lifeTime = CalculateLife(hash);
        particle.atlasIndex = CalculateAtlas(hash);
        particle.rotation = CalculateRotation(hash);
    } else {
        // Apply gravity to the particle velocity
        float3 gravity = float3(0, GRAVITY, 0) * EmitterConstants.gravity;
        particle.velocity += gravity * Camera.deltaTime;
        particle.position += particle.velocity * Camera.deltaTime;
        particle.lifeTime -= Camera.deltaTime;
        particle.color = ParticleColor(particle.lifeTime);
    }

    localParticles[groupIndex] = particle;

    // Ensure all threads have completed their work before writing back
    GroupMemoryBarrierWithGroupSync();
    Particles[particleIndex] = localParticles[groupIndex];
}
