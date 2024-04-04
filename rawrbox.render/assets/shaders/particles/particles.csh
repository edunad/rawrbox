#include "camera.fxh"
#include "particles_uniforms.fxh"
#include "hash.fxh"

#define WRITE_PARTICLES
#include "particles.fxh"

float3 CalculateVelocity(uint hash) {
    float3 randomVelocity;
    randomVelocity.x = lerp(EmitterConstants.velocity[0].x, EmitterConstants.velocity[1].x, (float)(hash & 0xFF) / 255.0f);
    randomVelocity.y = lerp(EmitterConstants.velocity[0].y, EmitterConstants.velocity[1].y, (float)((hash >> 8) & 0xFF) / 255.0f);
    randomVelocity.z = lerp(EmitterConstants.velocity[0].z, EmitterConstants.velocity[1].z, (float)((hash >> 16) & 0xFF) / 255.0f);

    return randomVelocity;
}

float2 CalculateSize(uint hash) {
    float2 randomSize;
    randomSize.x = lerp(EmitterConstants.size.x, EmitterConstants.size.z, (float)(hash & 0xFFFF) / 65535.0f);
    randomSize.y = lerp(EmitterConstants.size.y, EmitterConstants.size.w, (float)((hash >> 16) & 0xFFFF) / 65535.0f);
    return randomSize;
}

float CalculateLife(uint hash) {
    return lerp(EmitterConstants.life.x, EmitterConstants.life.y, (float)(hash & 0xFFFF) / 65535.0f);
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
    uint hash = pcg(EmitterConstants.time + particleIndex * 1009);

    // Calculate the spawn interval based on the spawn rate
    float spawnInterval = 1.0 / EmitterConstants.spawnRate;
    float particleSpawnTime = particleIndex / EmitterConstants.spawnRate;

    if (particle.lifeTime <= 0.0f && EmitterConstants.time >= particleSpawnTime) {
        // Spawn / reset a particle
        particle.position = EmitterConstants.position;
        particle.velocity = CalculateVelocity(hash);
        particle.size = CalculateSize(hash);
        particle.initialLifeTime = CalculateLife(hash);

        particle.lifeTime = particle.initialLifeTime;
        particle.color = EmitterConstants.color[0];
    } else {
        particle.position += particle.velocity * Camera.deltaTime;
        particle.lifeTime = max(particle.lifeTime - Camera.deltaTime, 0.0f);
        particle.color = ParticleColor(1.0f - particle.lifeTime / particle.initialLifeTime);
    }

    Particles[particleIndex] = particle;
}
