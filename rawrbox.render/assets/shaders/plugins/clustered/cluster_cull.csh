// Based off https://github.com/simco50/D3D12_Research <3

#include "camera.fxh"
#include "math.fxh"
#include "light_uniforms.fxh"
#include "decal_uniforms.fxh"

#define READ_LIGHTS
#define READ_DECALS
#define WRITE_CLUSTERS
#define WRITE_CLUSTER_DATA_GRID
#include "cluster.fxh"

struct Sphere {
	float3 Position;
	float Radius;
};

bool SphereInAABB(Sphere sphere, ClusterAABB aabb) {
	float3 d = max(0, abs(aabb.Center.xyz - sphere.Position) - aabb.Extents.xyz);
	float distanceSq = dot(d, d);

	return distanceSq <= sphere.Radius * sphere.Radius;
}

bool ConeInSphere(float3 conePosition, float3 coneDirection, float coneRange, float2 coneAngleSinCos, Sphere sphere) {
	float3 v = sphere.Position - conePosition;

	float lenSq = dot(v, v);
	float v1Len = dot(v, coneDirection);

	float distanceClosestPoint = coneAngleSinCos.y * sqrt(lenSq - v1Len * v1Len) - v1Len * coneAngleSinCos.x;

	bool angleCull = distanceClosestPoint > sphere.Radius;
	bool frontCull = v1Len > sphere.Radius + coneRange;
	bool backCull = v1Len < -sphere.Radius;

	return !(angleCull || frontCull || backCull);
}

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    if(any(dispatchThreadId >= GROUP_SIZE.xyz))
		return;

	uint clusterIndex = Flatten3D(dispatchThreadId, float2(CLUSTERS_X, CLUSTERS_Y));
    ClusterAABB cluster = Clusters[clusterIndex];

	float clusterRadius = sqrt(dot(cluster.Extents.xyz, cluster.Extents.xyz));

	uint lightIndex = 0;
	uint decalIndex = 0;

    float3 lightDir = mul(float4(0, 0, 0, 1.0), Camera.view).xyz;

    [loop]
    for(uint bucketIndex = 0; bucketIndex < CLUSTERED_NUM_BUCKETS && (lightIndex < TOTAL_LIGHTS || decalIndex < TOTAL_DECALS); ++bucketIndex) {
        ClusterData data = (ClusterData)0;

        [loop]
        for(uint i = 0; i < CLUSTERS_Z && lightIndex < TOTAL_LIGHTS; ++i) {
            Light light = Lights[lightIndex];
            ++lightIndex;

            if(light.type == LIGHT_POINT) {
                Sphere sphere;
                sphere.Radius = light.radius;
                sphere.Position = mul(light.position, Camera.view).xyz;

                if(SphereInAABB(sphere, cluster)) {
                    data.lights |= 1u << i;
                }
            } else if(light.type == LIGHT_SPOT) {
                float3 viewSpacePos = mul(light.position, Camera.view).xyz;
                float3 viewSpaceDir = mul(light.direction, Camera.view).xyz;
                viewSpaceDir = normalize(viewSpaceDir - lightDir);

                float2 coneAngleSinCos = float2(sin(light.umbra), cos(light.umbra)) + 0.3; // Add a bit of offset to the cull, so when you move the camera you don't notice the delay

                Sphere sphere;
				sphere.Radius = clusterRadius;
				sphere.Position = cluster.Center.xyz;

                if(ConeInSphere(viewSpacePos, viewSpaceDir, light.radius, coneAngleSinCos, sphere)) {
                    data.lights |= 1u << i;
                }
            } else if(light.type == LIGHT_DIRECTIONAL) {
                data.lights |= 1u << i; // Directional, don't calculate cull
            }
        }

        [loop]
        for(uint o = 0; o < CLUSTERS_Z && decalIndex < TOTAL_DECALS; ++o) {
            Decal decal = Decals[decalIndex];
            ++decalIndex;

            data.decals |= 1u << o; // TODO: CULLING
        }

	    ClusterDataGrid[clusterIndex * CLUSTERED_NUM_BUCKETS + bucketIndex] = data;
    }
}
