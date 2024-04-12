#ifndef INCLUDED_VERTEX_UNIFORMS
#define INCLUDED_VERTEX_UNIFORMS

#ifdef SKINNED
struct SkinnedConstantsStruct {
	// Model Bones ----
	float4x4 bones[MAX_BONES_PER_MODEL];
	// ----------------
};

ConstantBuffer<SkinnedConstantsStruct> SkinnedConstants;
#endif

struct ConstantsStruct {
	uint4 data;
	float4 dataF;
};

ConstantBuffer<ConstantsStruct> Constants;

#define ColorOverride Constants.data.x
#define SliceOverride Constants.data.y
#define GPUID         Constants.data.z
#define Billboard     Constants.data.w

#define VertexSnap          Constants.dataF.x
#define DisplacementTexture (uint) Constants.dataF.y
#define DisplacementPower   Constants.dataF.z

#endif
