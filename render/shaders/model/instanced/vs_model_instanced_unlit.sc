$input a_position, a_color0, a_texcoord0, i_data0, i_data1, i_data2, i_data3, i_data4, i_data5
$output v_color0, v_texcoord0

#include <bgfx_shader.sh>
#include <../../include/model_position.sh>

void main() {
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);
	vec4 worldPos = mul(model, vec4(a_position, 1.0) );

	gl_Position = mul(u_viewProj, worldPos);

	v_color0 = i_data4;
    v_texcoord0 = mix(i_data5.xy, i_data5.zw, a_texcoord0.xy);
}
