$input v_texcoord

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

void main() {
    vec4 color = texture2D(s_texColor, v_texcoord.xy);
    if(color.a <= 0.0) discard;

    gl_FragColor = color;
}
