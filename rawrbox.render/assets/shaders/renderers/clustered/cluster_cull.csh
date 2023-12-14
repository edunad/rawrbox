
cbuffer Constants {
    float4x4 g_View;
};

#include <light_uniforms.fxh>

#define READ_CLUSTERS
#define READ_LIGHTS
#define WRITE_CLUSTER_DATA_GRID
#include <cluster.fxh>

#include <light_utils.fxh>

bool SphereInAABB(Light light, ClusterAABB aabb) {
	float3 d = max(0, abs(aabb.Center.xyz - light.position) - aabb.Extents.xyz);
	float distanceSq = dot(d, d);

	return distanceSq <= (light.radius * light.radius);
}

bool ConeInSphere(Light light, ClusterAABB aabb) {
    float3 halfExtents = aabb.Extents.xyz * 0.5;
    float3 center = aabb.Center.xyz * 0.5;

    float sphereRadius = sqrt(dot(halfExtents, halfExtents));
    float3 v = center - light.position;

    float lenSq = dot(v, v);
    float v1Len = dot(v, light.direction);
    float cosAngle = cos(light.outerCone + 0.2);
    float sinAngle = sqrt(1.0 - cosAngle * cosAngle);

    float distanceClosestPoint = cosAngle * sqrt(lenSq - v1Len * v1Len) - v1Len * sinAngle;

    bool angleCull = distanceClosestPoint > sphereRadius;
    bool frontCull = v1Len > sphereRadius + light.radius;
    bool backCull = v1Len < -sphereRadius;

    return !(angleCull || frontCull || backCull);
}

/*bool LightIntersectsCluster(Light light, Cluster cluster) {
    if(light.type == LIGHT_SPOT) {  // Spot light
        float3 halfExtents = (cluster.maxBounds.xyz - cluster.minBounds.xyz) * 0.5;
        float3 center = (cluster.minBounds.xyz + cluster.maxBounds.xyz) * 0.5;

        float sphereRadius = sqrt(dot(halfExtents, halfExtents));
        float3 v = center - light.position;

        float lenSq = dot(v, v);
        float v1Len = dot(v, light.direction);
        float cosAngle = cos(light.outerCone + 0.2);
        float sinAngle = sqrt(1.0 - cosAngle * cosAngle);

        float distanceClosestPoint = cosAngle * sqrt(lenSq - v1Len * v1Len) - v1Len * sinAngle;

        bool angleCull = distanceClosestPoint > sphereRadius;
        bool frontCull = v1Len > sphereRadius + light.radius;
        bool backCull = v1Len < -sphereRadius;

        return !(angleCull || frontCull || backCull);
    } else { // Point light
        float3 closest = max(cluster.minBounds.xyz, min(light.position, cluster.maxBounds.xyz));
        float3 dist = closest - light.position;
        float lenSq = dot(dist, dist);

        return lenSq <= (light.radius * light.radius);
    }
}*/

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    uint3 clusterIndex3D = dispatchThreadId;
    if(any(clusterIndex3D >= GROUP_SIZE))
		return;

	uint clusterIndex = Flatten3D(clusterIndex3D, float2(CLUSTERS_X, CLUSTERS_Y));
    ClusterAABB cluster = g_Clusters[clusterIndex];

	float clusterRadius = sqrt(dot(cluster.Extents.xyz, cluster.Extents.xyz));

    uint lightCount = totalLights();
	uint lightIndex = 0;

    float3 lightDir = mul(float4(0, 0, 0, 1.0), g_View).xyz;

    // https://github.com/simco50/D3D12_Research/blob/master/D3D12/Resources/Shaders/ClusteredLightCulling.hlsl
    [loop]
    for(uint bucketIndex = 0; bucketIndex < CLUSTERED_LIGHTING_NUM_BUCKETS && lightIndex < lightCount; ++bucketIndex) {
		uint lightMask = 0;

        [loop]
        for(uint i = 0; i < 32 && lightIndex < lightCount; ++i) {
            Light light = g_Lights[lightIndex];
            light.position = mul(float4(light.position, 1.0), g_View).xyz;

            if(light.type == LIGHT_POINT) {
                if(SphereInAABB(light, cluster)) {
                    lightMask |= 1u << i;
                }
            } else if(light.type == LIGHT_SPOT) {
                light.direction = mul(float4(light.direction, 1.0), g_View).xyz;
                light.direction.xyz = normalize(light.direction - lightDir);

                if(ConeInSphere(light, cluster)) {
                    lightMask |= 1u << i;
                }
            } else {
                lightMask |= 1u << i; // Unknown, don't calculate cull
            }

            ++lightIndex;
        }

	    g_ClusterDataGrid[clusterIndex * CLUSTERED_LIGHTING_NUM_BUCKETS + bucketIndex] = lightMask;
    }
}
