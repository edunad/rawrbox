#ifndef UTIL_HEADER_GUARD
#define UTIL_HEADER_GUARD

float LinearizeDepth(float z, float near, float far) {
	return far / (far + z * (near - far));
}

float3 ViewPositionFromDepth(float2 uv, float depth, float2 nearFar, float4x4 invProj) {
	float4 clip = float4(float2(uv.x, 1.0f - uv.y) * 2.0f - 1.0f, 0.0f, 1.0f) * nearFar.x;
	float3 viewRay = mul(clip, invProj).xyz;

	return viewRay * LinearizeDepth(depth, nearFar.x, nearFar.y);
}

float3 ScreenToView(float4 screen, float4 invScreenSize, float2 nearFar, float4x4 invProj) {
	float2 screenNormalized = screen.xy * invScreenSize.zw;
    return ViewPositionFromDepth(screenNormalized, screen.z, nearFar, invProj);
}

uint Flatten2D(uint2 index, uint dimensionsX) {
	return index.x + index.y * dimensionsX;
}

uint Flatten3D(uint3 index, uint2 dimensionsXY) {
	return index.x + index.y * dimensionsXY.x + index.z * dimensionsXY.x * dimensionsXY.y;
}

uint2 UnFlatten2D(uint index, uint dimensionsX) {
	return uint2(index % dimensionsX, index / dimensionsX);
}

uint3 UnFlatten3D(uint index, uint2 dimensionsXY) {
	uint3 outIndex;
	outIndex.z = index / (dimensionsXY.x * dimensionsXY.y);
	index -= (outIndex.z * dimensionsXY.x * dimensionsXY.y);
	outIndex.y = index / dimensionsXY.x;
	outIndex.x = index % dimensionsXY.x;
	return outIndex;
}

uint DivideAndRoundUp(uint x, uint y) {
	return (x + y - 1) / y;
}
#endif // UTIL_HEADER_GUARD
