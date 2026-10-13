#ifndef INCLUDED_INSTANCE
#define INCLUDED_INSTANCE

struct Instance {
	float4 transform[3]; // Transposed matrix
	uint4 data;          // Color, slice, gpu id, ??
};

StructuredBuffer<Instance> Instances;

float3x4 GetInstanceMatrix(Instance instance) {
	return float3x4(instance.transform[0], instance.transform[1], instance.transform[2]);
}

#endif