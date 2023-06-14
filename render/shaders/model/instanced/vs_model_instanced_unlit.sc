$input a_position, a_color0, i_data0, i_data1, i_data2, i_data3, i_data4
$output v_color0

#include <bgfx_shader.sh>
#include <../../include/model_position.sh>

void main() {
	mat4 model = mtxFromCols(i_data0, i_data1, i_data2, i_data3);
	vec4 worldPos = mul(model, vec4(a_position, 1.0) );

	gl_Position = mul(u_viewProj, worldPos);
	v_color0 = vec4(1.0, 0.0, 0.0, 1.0);
}
