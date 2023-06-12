$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_position.sh>

void main() {
    gl_Position = getTranslatedPos(a_position, a_texcoord0);

    v_color0 = a_color0;
    v_texcoord0 = a_texcoord0;
}
