
#include <cluster_uniforms.fxh>
#include <light_uniforms.fxh>

#define READ_CLUSTERS
#define READ_LIGHTS
#define WRITE_ATOMIC
#define WRITE_LIGHT_INDICES
#define WRITE_LIGHT_GRID
#include <cluster.fxh>

#include <light_utils.fxh>

bool lightIntersectsCluster(Light light, Cluster cluster);

groupshared Light lights[THREAD_GROUP_SIZE];

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(uint3 GIid: SV_DispatchThreadID, uint GIndx: SV_GroupIndex) {

    // local thread variables
    // hold the result of light culling for this cluster
    uint visibleLights[MAX_LIGHTS_PER_CLUSTER];
    uint visibleCount = 0;

    // the way we calculate the index doesn't really matter here since we write to the same index in the light grid as we read from the cluster buffer
    uint clusterIndex = GIid.z * CLUSTERS_X_THREADS * CLUSTERS_Y_THREADS +
                        GIid.y * CLUSTERS_X_THREADS +
                        GIid.x;

    Cluster cluster = g_Clusters[clusterIndex];

    // we have a cache of THREAD_GROUP_SIZE lights
    // have to run this loop several times if we have more than THREAD_GROUP_SIZE lights
    uint lightCount = totalLights();
    uint lightOffset = 0;

    float3 lightDir = mul(float4(0, 0, 0, 1.0), g_Camera.view).xyz;

    while(lightOffset < lightCount) {
        // read THREAD_GROUP_SIZE lights into shared memory
        // each thread copies one light
        uint batchSize = min(THREAD_GROUP_SIZE, lightCount - lightOffset);
        if(GIndx < batchSize) {
            uint lightIndex = lightOffset + GIndx;

            Light light = getLight(lightIndex);
            // transform to view space (expected by pointLightAffectsCluster)
            // do it here once rather than for each cluster later
            light.position = mul(float4(light.position, 1.0), g_Camera.view).xyz;

            if(light.type == LIGHT_SPOT) {
                light.direction = mul(float4(light.direction, 1.0), g_Camera.view).xyz;
                light.direction.xyz = normalize(light.direction - lightDir);
            }

            lights[GIndx] = light;
        }

        // wait for all threads to finish copying
        GroupMemoryBarrierWithGroupSync(); // barrier();

        // each thread is one cluster and checks against all lights in the cache
        for(uint i = 0; i < batchSize; i++) {
            Cluster cluster = g_Clusters[clusterIndex];

            if(visibleCount < MAX_LIGHTS_PER_CLUSTER && lightIntersectsCluster(lights[i], cluster)) {
                visibleLights[visibleCount] = lightOffset + i;
                visibleCount++;
            }
        }

        lightOffset += batchSize;
    }

    // wait for all threads to finish checking lights
    GroupMemoryBarrierWithGroupSync(); //barrier();

    // get a unique index into the light index list where we can write this cluster's lights
    uint offset = 0;
    InterlockedAdd(g_globalIndex[0], visibleCount, offset);

    // copy indices of lights
    for(uint i = 0; i < visibleCount; i++) {
        g_clusterLightIndices[offset + i] = visibleLights[i];
    }

    // write light grid for this cluster
    g_clusterLightGrid[clusterIndex] = uint2(offset, visibleCount);
}


bool lightIntersectsCluster(Light light, Cluster cluster) {
    if(light.type == LIGHT_SPOT) {  // Spot light
        float3 halfExtents = (cluster.maxBounds - cluster.minBounds) * 0.5;
        float3 center = (cluster.minBounds + cluster.maxBounds) * 0.5;

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
        float3 closest = max(cluster.minBounds, min(light.position, cluster.maxBounds));
        float3 dist = closest - light.position;
        float lenSq = dot(dist, dist);

        return lenSq <= light.radius * light.radius; // check if point is inside the sphere
    }
}
