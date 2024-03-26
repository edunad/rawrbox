// Based off https://github.com/simco50/D3D12_Research <3

#include "camera.fxh"

#define WRITE_CLUSTERS
#include "cluster.fxh"

float GetDepthFromSlice(uint slice, float2 nearFar) {
	#ifdef VULKAN
		return nearFar.y * pow(abs(nearFar.x / nearFar.y), (float)slice / (float)CLUSTERS_Z);
	#else
		return nearFar.x * pow(abs(nearFar.y / nearFar.x), (float)slice / (float)CLUSTERS_Z);
	#endif
}

float3 LineFromOriginZIntersection(float3 lineFromOrigin, float depth) {
	float3 normal = float3(0.0f, 0.0f, 1.0f);
	float t = depth / dot(normal, lineFromOrigin);

	return t * lineFromOrigin;
}

ClusterAABB AABBFromMinMax(float3 minimum, float3 maximum) {
	ClusterAABB aabb;
	aabb.Center = float4((minimum + maximum) * 0.5, 0);
	aabb.Extents = float4(maximum, 0.0) - aabb.Center;
	return aabb;
}

ClusterAABB ComputeCluster(uint3 clusterIndex3D) {
	float2 minPoint_SS = float2(clusterIndex3D.x * CLUSTER_TEXTEL_SIZE, clusterIndex3D.y * CLUSTER_TEXTEL_SIZE);
	float2 maxPoint_SS = float2((clusterIndex3D.x + 1) * CLUSTER_TEXTEL_SIZE, (clusterIndex3D.y + 1) * CLUSTER_TEXTEL_SIZE);

	float2 screenInv = float2(1.0 / ScreenSize.x, 1.0 / ScreenSize.y);

	float3 minPoint_VS = ScreenToView(float3(minPoint_SS, 1.0), screenInv, NearFar, SCamera.projInv).xyz;
	float3 maxPoint_VS = ScreenToView(float3(maxPoint_SS, 1.0), screenInv, NearFar, SCamera.projInv).xyz;

	float farZ = GetDepthFromSlice(clusterIndex3D.z, NearFar);
	float nearZ = GetDepthFromSlice(clusterIndex3D.z + 1.0, NearFar);

	float3 minPointNear = LineFromOriginZIntersection(minPoint_VS, nearZ);
	float3 maxPointNear = LineFromOriginZIntersection(maxPoint_VS, nearZ);
	float3 minPointFar = LineFromOriginZIntersection(minPoint_VS, farZ);
	float3 maxPointFar = LineFromOriginZIntersection(maxPoint_VS, farZ);

	float3 bbMin = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
	float3 bbMax = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

	return AABBFromMinMax(bbMin, bbMax);
}

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    if(any(dispatchThreadId >= GROUP_SIZE.xyz))
		return;

	uint clusterIndex = Flatten3D(dispatchThreadId, float2(CLUSTERS_X, CLUSTERS_Y));
    Clusters[clusterIndex] = ComputeCluster(dispatchThreadId);
}
