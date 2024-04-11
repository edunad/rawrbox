#ifndef INCLUDED_PIXEL_UNIFORMS
#define INCLUDED_PIXEL_UNIFORMS

struct ConstantsStruct {
	uint4 textureIDs; // BASE, NORMAL, ROUGHTMETAL, EMISSION
	float4 litData;
};

ConstantBuffer<ConstantsStruct> Constants;

#define BaseID          Constants.textureIDs.x
#define NormalID        Constants.textureIDs.y
#define RoughtMetalID   Constants.textureIDs.z
#define EmissionID      Constants.textureIDs.w

#define RoughnessFactor Constants.litData.x
#define MetalnessFactor Constants.litData.y
#define SpecularFactor  Constants.litData.z
#define EmissionFactor  Constants.litData.w

#endif
