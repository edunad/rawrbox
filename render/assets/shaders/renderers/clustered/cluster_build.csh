
cbuffer Constants {
    float2   g_ZNearFarVec;
    float2   g_ClusterSize;

	// ------
    float4x4 g_InvProj;
};


#define ZNear g_ZNearFarVec.x
#define ZFar g_ZNearFarVec.y

#define WRITE_CLUSTERS
#include <cluster.fxh>

struct CSInput {
	uint3 GroupId : SV_GroupID;
	uint3 DispatchThreadId : SV_DispatchThreadID;
	uint3 GroupThreadId : SV_GroupThreadID;
	uint  GroupIndex : SV_GroupIndex;
};

[numthreads(1, 1, 1)]
void main(CSInput input) {
    uint tile_index = input.GroupId.x +
                      input.GroupId.y * CLUSTERS_X +
                      input.GroupId.z * (CLUSTERS_X * CLUSTERS_Y);

    uint2 tileSize = (uint2)g_ClusterSize.xy;

    float3 max_point_vs = GetViewPosition((input.GroupId.xy + 1) * tileSize, 1.0f, g_InvProj);
    float3 min_point_vs = GetViewPosition(input.GroupId.xy * tileSize, 1.0f, g_InvProj);

    float cluster_near = -ZNear * pow(ZFar / ZNear, input.GroupId.z / float(CLUSTERS_Z));
    float cluster_far = -ZNear * pow(ZFar / ZNear, (input.GroupId.z + 1) / float(CLUSTERS_Z));

    float3 minPointNear = IntersectionZPlane(min_point_vs, cluster_near);
    float3 minPointFar = IntersectionZPlane(min_point_vs, cluster_far);
    float3 maxPointNear = IntersectionZPlane(max_point_vs, cluster_near);
    float3 maxPointFar = IntersectionZPlane(max_point_vs, cluster_far);

    float3 minPointAABB = min(min(minPointNear, minPointFar), min(maxPointNear, maxPointFar));
    float3 maxPointAABB = max(max(minPointNear, minPointFar), max(maxPointNear, maxPointFar));

    g_Clusters[tile_index].minBounds = float4(minPointAABB, 0.0);
    g_Clusters[tile_index].maxBounds = float4(maxPointAABB, 0.0);
}
