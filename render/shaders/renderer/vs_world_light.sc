$input a_position

#include <bgfx_shader.sh>

void main(){
	gl_Position = vec4(a_position.xy, 1.0, 1.0);
}
