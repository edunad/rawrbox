// Based off https://github.com/simco50/D3D12_Research <3

#include <camera.fxh>

#define WRITE_CLUSTERS
#include <cluster.fxh>

float GetDepthFromSlice(uint slice, float2 nearFar) {
	return nearFar.x * pow(abs(nearFar.y / nearFar.x), (float)slice / (float)CLUSTERS_Z);
}

float3 LineFromOriginZIntersection(float3 lineFromOrigin, float depth) {
	float3 normal = float3(0.0f, 0.0f, 1.0f);
	float t = depth / dot(normal, lineFromOrigin);

	return t * lineFromOrigin;
}

ClusterAABB AABBFromMinMax(float3 minimum, float3 maximum) {
	ClusterAABB aabb;
	aabb.Center = float4((minimum + maximum) * 0.5, 0);
	aabb.Extents = float4(maximum, 0) - aabb.Center;
	return aabb;
}


ClusterAABB ComputeCluster(uint3 clusterIndex3D) {
	float2 minPoint_SS = float2(clusterIndex3D.x * CLUSTER_TEXTEL_SIZE, clusterIndex3D.y * CLUSTER_TEXTEL_SIZE);
	float2 maxPoint_SS = float2((clusterIndex3D.x + 1) * CLUSTER_TEXTEL_SIZE, (clusterIndex3D.y + 1) * CLUSTER_TEXTEL_SIZE);

	float2 nearFar = Camera.nearFar.xy;

	float3 minPoint_VS = ScreenToView(float4(minPoint_SS, 0, 1), Camera.viewportInv, nearFar, Camera.projInv).xyz;
	float3 maxPoint_VS = ScreenToView(float4(maxPoint_SS, 0, 1), Camera.viewportInv, nearFar, Camera.projInv).xyz;

	float farZ = GetDepthFromSlice(clusterIndex3D.z, nearFar);
	float nearZ = GetDepthFromSlice(clusterIndex3D.z + 1, nearFar);

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
    uint3 clusterIndex3D = dispatchThreadId;
    if(any(clusterIndex3D >= GROUP_SIZE))
		return;

	uint clusterIndex = Flatten3D(clusterIndex3D, float2(CLUSTERS_X, CLUSTERS_Y));
    Clusters[clusterIndex] = ComputeCluster(clusterIndex3D);
}
