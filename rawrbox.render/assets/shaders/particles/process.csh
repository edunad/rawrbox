
#include "particles_uniforms.fxh"

#define WRITE_PARTICLES
#include "particles.fxh"

float rand(uint2 input) {
    return frac(sin(dot(input, uint2(12.9898,78.233))) * 43758.5453);
}

[numthreads(256, 1, 1)]
void CSMain(uint3 DTid : SV_DispatchThreadID) {
    if (DTid.x >= TOTAL_PARTICLES) return;
    Particle p = GetParticle(DTid.x);

    if (p.data.x <= 0.0f) { // re-use particle
        p.position = EmitterConstants.position + normalize(float3(rand(DTid.xy), rand(DTid.yx), rand(DTid.xx)));
        p.velocity = normalize(p.position.xyz - EmitterConstants.position);
        p.data.x = EmitterConstants.lifetime;
    } else {
        p.velocity += EmitterConstants.velocity * EmitterConstants.deltaTime;
        p.position += p.velocity * EmitterConstants.deltaTime;
        p.data.x -= EmitterConstants.deltaTime; // Life
    }

    Particles[DTid.x] = p;
}
