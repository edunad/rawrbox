#ifndef INCLUDED_LIGHT_UNIFORMS
#define INCLUDED_LIGHT_UNIFORMS

#include "unpack.fxh"

struct LightConstantsStruct {
	// Light ---------
	uint4 lightSettings;
	// ------

	// Ambient ---
	float4 ambientColor;
	// -----
};

ConstantBuffer<LightConstantsStruct> LightConstants;

struct Light {
	float3 position;
	float radius;

	float3 direction;
	uint type;

    // SPOT LIGHT ----
	float cosUmbra;
	float cosPenumbra;
	// ----------------

	uint2 radiance; // RGB16F, color * intensity
};

float3 GetLightRadiance(Light light) {
	return float3(Unpack_RG16_FLOAT(light.radiance.x), f16tof32(light.radiance.y & 0xFFFFu));
}

// Aka sun
struct DirectionalLight {
	float4 direction;
	float4 radiance;
};

#define LIGHT_POINT       1
#define LIGHT_SPOT        2
#define LIGHT_DIRECTIONAL 3

#define FULL_BRIGHT  LightConstants.lightSettings.x
#define TOTAL_LIGHTS LightConstants.lightSettings.y
#endif
