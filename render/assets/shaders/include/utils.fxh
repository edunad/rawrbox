#ifndef UTIL_HEADER_GUARD
#define UTIL_HEADER_GUARD

float3 IntersectionZPlane(const float3 a, float z_dist) {
    float3 normal = float3(0.0, 0.0, -1.0);
    float t = z_dist / a.z; //dot(normal, d);

    float3 result = t * a;
    return result;

}

float GetLinearDepth(float depth, float2 camNearFar) {
    float z_b = depth;
    float z_n = 2.0 * z_b - 1.0;
    float linearDepth = 2.0 * camNearFar.x * camNearFar.y / (camNearFar.y + camNearFar.x - z_n * (camNearFar.y - camNearFar.x));

    return linearDepth;
}

float3 GetViewPosition(float2 texcoord, float depth, float4x4 invProj) {
	float4 ndc = float4(texcoord * 2.0f - 1.0f, depth, 1.0f);
    ndc.y *= -1;
    float4 wp = mul(ndc, invProj);
    return wp.xyz / wp.w;
}

#endif // UTIL_SH_HEADER_GUARD
