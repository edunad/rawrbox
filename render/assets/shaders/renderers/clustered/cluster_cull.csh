
cbuffer Constants {
    float4x4 g_View;
};

#include <light_uniforms.fxh>

#define READ_CLUSTERS
#define READ_LIGHTS
#define WRITE_ATOMIC
#define WRITE_LIGHT_INDICES
#define WRITE_CLUSTER_DATA_GRID
#include <cluster.fxh>

#include <light_utils.fxh>

struct CSInput {
	uint3 GroupId : SV_GroupID;
	uint3 DispatchThreadId : SV_DispatchThreadID;
	uint3 GroupThreadId : SV_GroupThreadID;
	uint  GroupIndex : SV_GroupIndex;
};

groupshared Light sharedLights[MAX_LIGHTS_PER_CLUSTER];

bool LightIntersectsCluster(Light light, Cluster cluster) {
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
}

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(CSInput input) {
    if (all(input.DispatchThreadId == 0)) { // Reset atomic index
		g_AtomicIndex[0] = 0;
	}

    uint visibleLightCount = 0;
    uint visibleLightIndices[MAX_LIGHTS_PER_CLUSTER];

	uint clusterIndex = input.GroupIndex + THREAD_GROUP_SIZE * input.GroupId.z;
    Cluster cluster = g_Clusters[clusterIndex];

    uint lightCount = totalLights();
    uint lightOffset = 0;

    float3 lightDir = mul(float4(0, 0, 0, 1.0), g_View).xyz;

    while (lightOffset < lightCount) {
		uint batchSize = min(THREAD_GROUP_SIZE, lightCount - lightOffset);

        if (input.GroupIndex < batchSize) {
            uint lightIndex = lightOffset + input.GroupIndex;

			Light light = g_Lights[lightIndex];

            light.position = mul(float4(light.position, 1.0), g_View).xyz;
            if(light.type == LIGHT_SPOT){
                light.direction = mul(float4(light.direction, 1.0), g_View).xyz;
                light.direction.xyz = normalize(light.direction - lightDir);
            }

			sharedLights[input.GroupIndex] = light;
		}

		GroupMemoryBarrierWithGroupSync();

		for (uint i = 0; i < batchSize; i++)
		{
			if (visibleLightCount < MAX_LIGHTS_PER_CLUSTER && LightIntersectsCluster(sharedLights[i], cluster))
			{
				visibleLightIndices[visibleLightCount] = lightOffset + i;
				visibleLightCount++;
			}
		}

		lightOffset += batchSize;
    }

	GroupMemoryBarrierWithGroupSync();

	uint lOffset = 0;
	InterlockedAdd(g_AtomicIndex[0], visibleLightCount, lOffset);

    for (uint i = 0; i < visibleLightCount; i++) {
		g_ClusterLightIndices[lOffset + i] = visibleLightIndices[i];
	}

    g_ClusterDataGrid[clusterIndex] = uint4(lOffset, visibleLightCount, 0, 0);
}
