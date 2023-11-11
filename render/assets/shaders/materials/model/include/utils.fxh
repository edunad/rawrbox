#ifndef UTIL_HEADER_GUARD
#define UTIL_HEADER_GUARD

float4 screen2Eye(float4 coord) {
    float3 ndc = float3(
        2.0 * (coord.x - g_Camera.screenSize.x) / g_Camera.screenSize.z - 1.0,
        2.0 * (g_Camera.screenSize.w - coord.y - 1 - g_Camera.screenSize.y) / g_Camera.screenSize.w - 1.0, // y is flipped
        coord.z // -> [0, 1]
    );

    // https://stackoverflow.com/a/16597492/862300
    float4 eye = mul(float4(ndc, 1.0), g_Camera.invProj);
    eye = eye / eye.w;
    return eye;
}


// https://stackoverflow.com/a/45710371/862300
float screen2EyeDepth(float depth, float near, float far) {
    float ndc = depth;
    float eye = far * near / (far + ndc * (near - far));
    return eye;
}

#endif // UTIL_SH_HEADER_GUARD
