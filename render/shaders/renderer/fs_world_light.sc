#include <bgfx_shader.sh>

#define READ_G_BUFFER

#include "../include/material.sh"
#include "../include/lights.sh"

void main() {
    vec2 texcoord = gl_FragCoord.xy / u_viewRect.zw;

    vec3 diffuseColor = texture2D(s_texDiffuse, texcoord).rgb;
    vec4 emissiveSpec = texture2D(s_texEmissionSpec, texcoord);

    vec3 radianceOut = vec3_splat(0.0);
    radianceOut += getAmbientLight() * diffuseColor;
    radianceOut += emissiveSpec.xyz;

    gl_FragColor = vec4(radianceOut, 1.0);
}
