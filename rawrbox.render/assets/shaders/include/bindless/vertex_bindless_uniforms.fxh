#ifndef INCLUDED_VERTEX_UNIFORMS
#define INCLUDED_VERTEX_UNIFORMS

#ifdef SKINNED
struct SkinnedConstantsStruct {
	// Model Bones ----
	float4x4 bones[MAX_BONES_PER_MODEL];
	// ----------------
};
#endif

struct ConstantsStruct {
	// Model ----
	float4 colorOverride;
	float4 data;
	// ----------------

	float4 gpuID;
};

#ifdef SKINNED
ConstantBuffer<SkinnedConstantsStruct> SkinnedConstants;
#endif

ConstantBuffer<ConstantsStruct> Constants;

#define Billboard           (uint) Constants.data.x
#define VertexSnap          Constants.data.y
#define DisplacementTexture (uint) Constants.data.z
#define DisplacementPower   Constants.data.w
#endif
