// Based off https://github.com/simco50/D3D12_Research <3

#ifdef INCLUDED_LIGHT_UNIFORMS
	#ifdef READ_LIGHTS
		#ifdef READ_TILES

			#ifndef INCLUDED_LIGHTING
				#define INCLUDED_LIGHTING
				#include "include/math.fxh"

struct LightResult {
	float3 Diffuse;
	float3 Specular;
};

Light GetLight(uint index) {
	return Lights[NonUniformResourceIndex(index)];
}

float3 Diffuse_Lambert(float3 diffuseColor) {
	return diffuseColor * INV_PI;
}

// Angle >= Umbra -> 0
// Angle < Penumbra -> 1
// Gradient between Umbra and Penumbra
float DirectionalSpotAttenuation(float3 L, float3 direction, float cosUmbra, float cosPenumbra) {
	float cosAngle = dot(-normalize(L), direction);
	float falloff = saturate((cosAngle - cosUmbra) / max(cosPenumbra - cosUmbra, 1e-4)); // Equal cones = hard edge
	return falloff * falloff;
}

// https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

float GetDistanceFalloff(float distSq) {
	return 1.0 / (distSq + 1.0);
}

float GetRangeWindow(float distSq, float range) {
	return Square(saturate(1.0 - Square(distSq * Square(rcp(range)))));
}

float RadialAttenuation(float3 L, float range) {
	float distSq = dot(L, L);
	return GetDistanceFalloff(distSq) * GetRangeWindow(distSq, range);
}

// RECT LIGHTS ---
struct RectLight {
	float3 center;
	float3 normal;
	float3 right;
	float3 up;
	float2 extent;
};

RectLight GetRectLight(Light light) {
	RectLight rect;
	rect.center = light.position;
	rect.normal = light.direction;
	rect.extent = Unpack_RG16_FLOAT(asuint(light.cosUmbra));

	float3 right = UnpackOctNormal(Unpack_RG16_FLOAT(asuint(light.cosPenumbra)));
	rect.right = normalize(right - rect.normal * dot(right, rect.normal));
	rect.up = cross(rect.normal, rect.right);

	return rect;
}

float3 GetRectClosestPoint(RectLight rect, float3 pos) {
	float3 offset = pos - rect.center;
	float2 uv = clamp(float2(dot(offset, rect.right), dot(offset, rect.up)), -rect.extent, rect.extent);
	return rect.center + rect.right * uv.x + rect.up * uv.y;
}

float3 GetRectRayPoint(RectLight rect, float3 origin, float3 dir) {
	float denom = dot(dir, rect.normal);
	float t = abs(denom) > 1e-4 ? dot(rect.center - origin, rect.normal) / denom : -1.0;

	float3 hit = t > 0.0 ? origin + dir * t : origin + dir * max(dot(rect.center - origin, dir), 0.0);
	return GetRectClosestPoint(rect, hit);
}
// ------

float GetAttenuation(Light light, float3 worldPosition, out float3 L) {
	// RECT LIGHTS
	if (GetLightType(light) == LIGHT_RECT) {
		RectLight rect = GetRectLight(light);
		float3 offset = worldPosition - rect.center;

		L = -rect.normal;
		if (dot(offset, rect.normal) <= 1e-4) return 0.0;

		float3 toLight = GetRectClosestPoint(rect, worldPosition) - worldPosition;
		float distSq = max(dot(toLight, toLight), 1e-6);
		L = toLight * rsqrt(distSq);

		return GetDistanceFalloff(distSq) * GetRangeWindow(dot(offset, offset), light.radius) * saturate(dot(rect.normal, -L));
	}
	//--------------------

	// Directional (sun)
	if (GetLightType(light) == LIGHT_DIRECTIONAL) {
		float lenSq = dot(light.direction, light.direction);
		L = lenSq > 0.0f ? -light.direction * rsqrt(lenSq) : float3(0, 0, 0); // Zero direction = no light

		return 1.0f;
	}
	//--------------------

	L = light.position - worldPosition;
	float attenuation = RadialAttenuation(L, light.radius);

	if (GetLightType(light) == LIGHT_SPOT) {
		attenuation *= DirectionalSpotAttenuation(L, light.direction, light.cosUmbra, light.cosPenumbra);
	}

	float distSq = dot(L, L);
	L *= rsqrt(distSq);

	return attenuation;
}

// GGX / Trowbridge-Reitz
// Note the division by PI here
// [Walter et al. 2007, "Microfacet models for refraction through rough surfaces"]
float D_GGX(float a2, float NdotH) {
	float d = (NdotH * a2 - NdotH) * NdotH + 1;
	return a2 / (PI * d * d);
}

// Appoximation of joint Smith term for GGX
// Returned value is G2 / (4 * NdotL * NdotV). So predivided by specular BRDF denominator
// [Heitz 2014, "Understanding the Masking-Shadowing Function in Microfacet-Based BRDFs"]
float Vis_SmithJointApprox(float a2, float NdotV, float NdotL) {
	float Vis_SmithV = NdotL * (NdotV * (1 - a2) + a2);
	float Vis_SmithL = NdotV * (NdotL * (1 - a2) + a2);
	return 0.5 * rcp(Vis_SmithV + Vis_SmithL);
}

// [Schlick 1994, "An Inexpensive BRDF Model for Physically-Based Rendering"]
float3 F_Schlick(float3 f0, float VdotH) {
	float Fc = Pow5(1.0f - VdotH);
	return Fc + (1.0f - Fc) * f0;
}

float3 F_Schlick(float3 f0, float3 f90, float VdotH) {
	float Fc = Pow5(1.0f - VdotH);
	return f90 * Fc + (1.0f - Fc) * f0;
}

// 0.08 is a max F0 we define for dielectrics which matches with Crystalware and gems (0.05 - 0.08)
// This means we cannot represent Diamond-like surfaces as they have an F0 of 0.1 - 0.2
float DielectricSpecularToF0(float specular) {
	return 0.08f * specular;
}

// Narkowicz2014 - Analytical DFG Term for IBL
// Source: https://knarkowicz.wordpress.com/2014/12/27/analytical-dfg-term-for-ibl/
float3 EnvDFGPolynomial(float3 specularColor, float gloss, float ndotv) {
	float x = gloss;
	float y = ndotv;

	float b1 = -0.1688;
	float b2 = 1.895;
	float b3 = 0.9903;
	float b4 = -4.853;
	float b5 = 8.404;
	float b6 = -5.069;
	float bias = saturate(min(b1 * x + b2 * x * x, b3 + b4 * y + b5 * y * y + b6 * y * y * y));

	float d0 = 0.6045;
	float d1 = 1.699;
	float d2 = -0.5228;
	float d3 = -3.603;
	float d4 = 1.404;
	float d5 = 0.1939;
	float d6 = 2.661;
	float delta = saturate(d0 + d1 * x + d2 * y + d3 * x * x + d4 * x * y + d5 * y * y + d6 * x * x * x);
	float scale = delta - bias;

	bias *= saturate(50.0 * specularColor.y);
	return specularColor * scale + bias;
}

// Note from Filament: vec3 f0 = 0.16 * reflectance * reflectance * (1.0 - metallic) + baseColor * metallic;
// F0 is the base specular reflectance of a surface
// For dielectrics, this is monochromatic commonly between 0.02 (water) and 0.08 (gems) and derived from a separate specular value
// For conductors, this is based on the base color we provided
float3 ComputeF0(float specular, float3 baseColor, float metalness) {
	return lerp(DielectricSpecularToF0(specular).xxx, baseColor, metalness);
}

float3 ComputeDiffuseColor(float3 baseColor, float metalness) {
	if (metalness == 1.0) {
		return baseColor;
	} else {
		return baseColor * (1 - metalness);
	}
}

LightResult DefaultLitBxDF(float3 specularColor, float specularRoughness, float3 diffuseColor, half3 N, half3 V, half3 L, float falloff) {
	LightResult lighting = (LightResult)0;

	if (falloff <= 0.0f) {
		return lighting;
	} else {
		float NdotL = saturate(dot(N, L));
		if (NdotL == 0.0f) {
			return lighting;
		} else {
			if (specularColor.r != 0.0 && specularColor.g != 0.0 && specularColor.b != 0.0) { // No specular texture?
				float3 H = normalize(V + L);
				float NdotV = saturate(abs(dot(N, V)) + 1e-5); // Bias to avoid artifacting
				float NdotH = saturate(dot(N, H));
				float VdotH = saturate(dot(V, H));

				// Generalized microfacet Specular BRDF
				float a = Square(specularRoughness);
				float a2 = clamp(Square(a), 0.0001f, 1.0f);
				float D = D_GGX(a2, NdotH);
				float Vis = Vis_SmithJointApprox(a2, NdotV, NdotL);
				float3 F = F_Schlick(specularColor, VdotH);

				lighting.Specular = (falloff * NdotL) * (D * Vis) * F;

				// Kulla17 - Energy conervation due to multiple scattering
				/*float gloss = Pow4(1 - specularRoughness);
				float3 DFG = EnvDFGPolynomial(specularColor, gloss, NdotV);
				float3 energyCompensation = 1.0f + specularColor * (1.0f / DFG.y - 1.0f);
				lighting.Specular *= energyCompensation;*/
			}

			// Diffuse BRDF
			lighting.Diffuse = (falloff * NdotL) * Diffuse_Lambert(diffuseColor);
			return lighting;
		}
	}
}

void ApplyLight(uint index, inout LightResult lighting, float3 specular, float R, float3 diffuse, float3 N, float3 geoNormal, float3 V, float3 worldPos, float dither) {
	if (TOTAL_LIGHTS == 0) return;
	Light light = GetLight(index);

	// RECT ---
	if (GetLightType(light) == LIGHT_RECT) {
		RectLight rect = GetRectLight(light);

		float3 offset = worldPos - rect.center;
		if (dot(offset, rect.normal) <= 1e-4) return;

		float window = GetRangeWindow(dot(offset, offset), light.radius);
		if (window <= 0.0) return;

		// Diffuse
		float3 toDiffuse = 0.5 * (GetRectClosestPoint(rect, worldPos) + GetRectRayPoint(rect, worldPos, N)) - worldPos;
		float diffuseDistSq = max(dot(toDiffuse, toDiffuse), 1e-6);
		float3 Ld = toDiffuse * rsqrt(diffuseDistSq);

		float wrap = saturate(max(rect.extent.x, rect.extent.y) * rsqrt(diffuseDistSq));
		float NdotLd = saturate((dot(N, Ld) + wrap) / (1.0 + wrap));
		float diffuseFalloff = window * GetDistanceFalloff(diffuseDistSq) * saturate(dot(rect.normal, -Ld));

		// Specular
		float3 toSpecular = GetRectRayPoint(rect, worldPos, reflect(-V, N)) - worldPos;
		float specularDistSq = max(dot(toSpecular, toSpecular), 1e-6);
		float3 Ls = toSpecular * rsqrt(specularDistSq);

		float a = max(Square(R), 0.01);
		float sourceRadius = 2.0 * sqrt(rect.extent.x * rect.extent.y * INV_PI);
		float energy = Square(a / saturate(a + 0.5 * saturate(sourceRadius * rsqrt(specularDistSq))));
		float specularFalloff = window * GetDistanceFalloff(specularDistSq) * saturate(dot(rect.normal, -Ls)) * energy;

	#ifdef READ_SHADOWS
		float shadow = GetShadow(light, worldPos, geoNormal, dither);
		diffuseFalloff *= shadow;
		specularFalloff *= shadow;
	#endif

		float3 radiance = GetLightRadiance(light);
		lighting.Diffuse += (diffuseFalloff * NdotLd) * Diffuse_Lambert(diffuse) * radiance;
		lighting.Specular += DefaultLitBxDF(specular, R, diffuse, N, V, Ls, specularFalloff).Specular * radiance;
		return;
	}
	// ------

	float3 L;
	float attenuation = GetAttenuation(light, worldPos, L);

#ifdef READ_SHADOWS
	if (attenuation > 0.0F) attenuation *= GetShadow(light, worldPos, geoNormal, dither);
#endif

	if (attenuation > 0.0F) {
		LightResult result = DefaultLitBxDF(specular, R, diffuse, N, V, L, attenuation);
		float3 radiance = GetLightRadiance(light);

		lighting.Diffuse += result.Diffuse * radiance;
		lighting.Specular += result.Specular * radiance;
	}
}

void ApplyLights(uint mask, uint bin, inout LightResult lighting, float3 specular, float R, float3 diffuse, float3 N, float3 geoNormal, float3 V, float3 worldPos, float dither) {
	if (TOTAL_LIGHTS == 0) return;

	while (mask) {
		uint bitIndex = firstbitlow(mask);
		mask ^= 1u << bitIndex;

		uint index = bitIndex + bin * BIN_BITS;
		if (index >= TOTAL_LIGHTS) break;

		ApplyLight(index, lighting, specular, R, diffuse, N, geoNormal, V, worldPos, dither);
	}
}

			#endif

		#endif
	#endif
#endif