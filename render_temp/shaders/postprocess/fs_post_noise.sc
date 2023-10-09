$input v_texcoord

#include <bgfx_shader.sh>
#include <../include/hash.sh>

uniform vec2 u_settings;

#define timer u_settings.x
#define strength u_settings.y

SAMPLER2D(s_texColor, 0);

uvec3 hash(vec2 s) {
	uvec4 u = uvec4(s, uint(s.x) ^ uint(s.y), uint(s.x) + uint(s.y));
    return pcg3d(u.xyz);
}

void main() {
	vec4 col = texture2D(s_texColor, v_texcoord.xy);
	if(col.a <= 0.0) discard;

	uvec3 h = hash(v_texcoord * 3000.0 * timer);
	float a = float(h.x) * (1.0 / float(0xffffffffu));
    vec3 noise = vec3(a, a, a) * strength;

	gl_FragColor = vec4(col.rgb - noise.xyz, col.a);
}
