$input v_texcoord0

#include <bgfx_shader.sh>

SAMPLER2D(s_texColor, 0);

uniform vec2 u_intensity;

#define px (vec2(1.0, 1.0) / u_viewRect.zw)
vec4 GetBloom ( in vec2 uv, in vec4 inColor ) {
	float numSamples = 1.0;
    vec4 color = inColor;

	for (float x = -8.0; x <= 8.0; x += 1.0) {
		for (float y = -8.0; y <= 8.0; y += 1.0) {
			vec4 addColor = texture2D(s_texColor, uv + (vec2(x, y) * px));
			if (max(addColor.r, max(addColor.g, addColor.b)) > 0.3) {
				float dist = length(vec2(x,y))+1.0;
				vec4 glowColor = max((addColor * 128.0) / pow(dist, 2.0), vec4(0,0,0,0));

				if (max(glowColor.r, max(glowColor.g, glowColor.b)) > 0.0) {
					color += glowColor;
					numSamples += 1.0;
				}
			}
		}
	}

	return color / numSamples;
}

// Adapted from https://www.shadertoy.com/view/4tlBWH
void main() {
    vec4 color =  texture2D(s_texColor, v_texcoord0.xy);
	if(color.a <= 0.0) discard;

    gl_FragColor = mix(color, GetBloom(v_texcoord0.xy, color), u_intensity.x);
}
