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

	uint2 radiance; // RGB16F, linear -> color * intensity
};

float3 GetLightRadiance(Light light) {
	return float3(Unpack_RG16_FLOAT(light.radiance.x), f16tof32(light.radiance.y & 0xFFFFu));
}

#define LIGHT_POINT       1
#define LIGHT_SPOT        2
#define LIGHT_DIRECTIONAL 3

#define LIGHT_TYPE_MASK    0x3u
#define LIGHT_SHADOW_SHIFT 2u

uint GetLightType(Light light) {
	return light.type & LIGHT_TYPE_MASK;
}

uint GetLightShadow(Light light) {
	return light.type >> LIGHT_SHADOW_SHIFT;
}

#define FULL_BRIGHT              LightConstants.lightSettings.x
#define TOTAL_LIGHTS             LightConstants.lightSettings.y
#define TOTAL_DIRECTIONAL_LIGHTS LightConstants.lightSettings.z
#define LIGHT_DEBUG              LightConstants.lightSettings.w
#endif
