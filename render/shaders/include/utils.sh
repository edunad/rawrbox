// Adapted from: https://github.com/pezcode/Cluster ♥

#ifndef UTIL_SH_HEADER_GUARD
#define UTIL_SH_HEADER_GUARD

#include <bgfx_shader.sh>

// from screen coordinates (gl_FragCoord) to eye space
vec4 screen2Eye(vec4 coord) {
#if BGFX_SHADER_LANGUAGE_GLSL
    // https://www.khronos.org/opengl/wiki/Compute_eye_space_from_window_space
    vec3 ndc = vec3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (coord.y - u_viewRect.y) / u_viewRect.w - 1.0,
        2.0 * coord.z - 1.0 // -> [-1, 1]
    );
#else
    vec3 ndc = vec3(
        2.0 * (coord.x - u_viewRect.x) / u_viewRect.z - 1.0,
        2.0 * (u_viewRect.w - coord.y - 1 - u_viewRect.y) / u_viewRect.w - 1.0, // y is flipped
        coord.z // -> [0, 1]
    );
#endif

    // https://stackoverflow.com/a/16597492/862300
    vec4 eye = mul(u_invProj, vec4(ndc, 1.0));
    eye = eye / eye.w;
    return eye;
}

// depth from screen coordinates (gl_FragCoord.z) to eye space
// same as screen2Eye(vec4(0, 0, depth, 1)).z but slightly faster
// (!) this assumes a perspective projection as created by bx::mtxProj
// for a left-handed coordinate system
// https://stackoverflow.com/a/45710371/862300
float screen2EyeDepth(float depth, float near, float far) {
#if BGFX_SHADER_LANGUAGE_GLSL
    float ndc = 2.0 * depth - 1.0;
    float eye = 2.0 * far * near / (far + near + ndc * (near - far));
#else
    float ndc = depth;
    float eye = far * near / (far + ndc * (near - far));
#endif
    return eye;
}

vec3 convertTangentNormal(vec3 normal_ref, vec3 tangent_ref, vec3 matNormal) {
    mat3 TBN = mtxFromCols(
        normalize(tangent_ref),
        normalize(cross(normal_ref, tangent_ref)),
        normalize(normal_ref)
    );

    return normalize(mul(TBN, matNormal));
}
#endif // UTIL_SH_HEADER_GUARD
