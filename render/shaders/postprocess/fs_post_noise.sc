$input v_texcoord0

#include <bgfx_shader.sh>
#include <../include/hash.sh>

uniform vec2 u_settings;
SAMPLER2D(s_texColor, 0);

uvec3 hash(vec2 s) {
	uvec4 u = uvec4(s, uint(s.x) ^ uint(s.y), uint(s.x) + uint(s.y)); // Play with different values for 3rd and 4th params. Some hashes are okay with constants, most aren't.
    return pcg3d(u.xyz);
}

void main() {
	vec4 col = texture2D(s_texColor, v_texcoord0.xy);
	if(col.a <= 0.01) discard;

	uvec3 h = hash(v_texcoord0 * 3000.0 * u_settings.x); // x = timer
	float a = float(h.x) * (1.0 / float(0xffffffffu));
    vec3 noise = vec3(a, a, a) * u_settings.y; // y = strength

	gl_FragColor = vec4(col.rgb - noise.xyz, col.a);
}
