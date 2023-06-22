$input a_position, a_color0, a_texcoord0
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_transforms.sh>

void main() {
    v_color0 = a_color0;
    v_texcoord0 = a_texcoord0.xy;

    gl_Position = applyPosTransforms(a_position, v_texcoord0);
}
