// Adapted from: https://github.com/pezcode/Cluster ♥

#ifndef UTIL_HEADER_GUARD
#define UTIL_HEADER_GUARD

// from screen coordinates (gl_FragCoord) to eye space
float4 screen2Eye(float4 coord) {
    float3 ndc = float3(
        2.0 * (coord.x - g_ScreenSize.x) / g_ScreenSize.z - 1.0,
        2.0 * (g_ScreenSize.w - coord.y - 1 - g_ScreenSize.y) / g_ScreenSize.w - 1.0, // y is flipped
        coord.z // -> [0, 1]
    );

    // https://stackoverflow.com/a/16597492/862300
    float4 eye = mul(float4(ndc, 1.0), g_InvProj);
    eye = eye / eye.w;
    return eye;
}



// depth from screen coordinates (gl_FragCoord.z) to eye space
// same as screen2Eye(vec4(0, 0, depth, 1)).z but slightly faster
// (!) this assumes a perspective projection as created by bx::mtxProj
// for a left-handed coordinate system
// https://stackoverflow.com/a/45710371/862300
float screen2EyeDepth(float depth, float near, float far) {
// if BGFX_SHADER_LANGUAGE_GLSL
//    float ndc = 2.0 * depth - 1.0;
//    float eye = 2.0 * far * near / (far + near + ndc * (near - far));
// else
//    float ndc = depth;
//    float eye = far * near / (far + ndc * (near - far));
// endif

    float ndc = depth;
    float eye = far * near / (far + ndc * (near - far));
    return eye;
}



#endif // UTIL_SH_HEADER_GUARD
