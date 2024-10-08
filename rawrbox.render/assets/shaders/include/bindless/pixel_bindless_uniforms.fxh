#ifndef INCLUDED_PIXEL_UNIFORMS
#define INCLUDED_PIXEL_UNIFORMS

struct ConstantsStruct {
	uint4 textureIDs; // BASE, NORMAL, ROUGHTMETAL, EMISSION
	float4 textureData;
	float4 pixelData;
};

ConstantBuffer<ConstantsStruct> Constants;

#define BaseID        Constants.textureIDs.x
#define NormalID      Constants.textureIDs.y
#define RoughtMetalID Constants.textureIDs.z
#define EmissionID    Constants.textureIDs.w

#define RoughnessFactor Constants.textureData.x
#define MetalnessFactor Constants.textureData.y
#define SpecularFactor  Constants.textureData.z
#define EmissionFactor  Constants.textureData.w

#define AlphaCutoff  Constants.pixelData.x

#endif
