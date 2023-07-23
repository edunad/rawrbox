$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
    vec3 color = texture2D(s_texColor, v_texcoord0.xy).rgb;
    gl_FragColor = vec4(color, 1.0);
}
