$input v_normal, v_tangent, v_texcoord, v_gpuPick, v_color0, v_worldPos, v_data

#define READ_CLUSTERS

#include <bgfx_shader.sh>
#include "../../include/clusters.sh"

void main() {
    // colorize clusters
    ARRAY_BEGIN(vec3, colors, 6)
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0),
        vec3(1.0, 1.0, 0.0),
        vec3(0.0, 1.0, 1.0),
        vec3(1.0, 0.0, 1.0)
    ARRAY_END();

    uint cluster = getClusterZIndex(gl_FragCoord.z);
    vec3 sliceColor = colors[cluster % 6];
    gl_FragColor = vec4(sliceColor, 1.0);
}
