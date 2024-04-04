#include "camera.fxh"
#include "particles_uniforms.fxh"
#include "hash.fxh"

#define WRITE_PARTICLES
#include "particles.fxh"

float3 CalculateVelocity(uint hash) {
    float t = (float)hash / 4294967295.0f;
    return lerp(EmitterConstants.velocity[0].xyz, EmitterConstants.velocity[1].xyz, t);
}

float2 CalculateSize(uint hash) {
    float t = (float)hash / 4294967295.0f;
    return lerp(EmitterConstants.size.xy, EmitterConstants.size.zw, t);
}

float4 ParticleColor(float lifetimeRatio) {
    if (lifetimeRatio < 0.33f) {
        return lerp(EmitterConstants.color[0], EmitterConstants.color[1], lifetimeRatio * 3.0f);
    } else if (lifetimeRatio < 0.66f) {
        return lerp(EmitterConstants.color[1], EmitterConstants.color[2], (lifetimeRatio - 0.33f) * 3.0f);
    } else {
        return lerp(EmitterConstants.color[2], EmitterConstants.color[3], (lifetimeRatio - 0.66f) * 3.0f);
    }
}

[numthreads(255, 1, 1)]
void main(uint3 dispatchThreadID : SV_DispatchThreadID) {
    uint particleIndex = dispatchThreadID.x;
    if (particleIndex >= EmitterConstants.maxParticles) return;

    Particle particle = GetParticle(particleIndex);

    //uint3 hash = pcg3d16(uint3(EmitterConstants.time, EmitterConstants.time * 7, EmitterConstants.time * 13));
    uint hashForSize = pcg(EmitterConstants.time * 7);

    // Calculate the spawn interval based on the spawn rate
    float spawnInterval = 1.0 / EmitterConstants.spawnRate;
    float particleSpawnTime = particleIndex / EmitterConstants.spawnRate;

    if (particle.lifeTime <= 0.0f && EmitterConstants.time >= particleSpawnTime) {
        // Spawn / reset a particle
        particle.position = EmitterConstants.position;
        particle.velocity = CalculateVelocity(hashForSize);
        particle.size = CalculateSize(hashForSize);
        particle.initialLifeTime = lerp(EmitterConstants.life.x, EmitterConstants.life.y, hashForSize);
        particle.lifeTime = particle.initialLifeTime;
        particle.color = EmitterConstants.color[0];
    } else {
        particle.position += particle.velocity * Camera.deltaTime;
        particle.lifeTime = max(particle.lifeTime - Camera.deltaTime, 0.0f);
        particle.color = ParticleColor(1.0f - particle.lifeTime / particle.initialLifeTime);
    }

    Particles[particleIndex] = particle;
}
