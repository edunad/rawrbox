$input v_texcoord

#include <bgfx_shader.sh>
#include <../include/dither_psx.sh>

SAMPLER2D(s_texColor, 0);
SAMPLER2D(s_ditherColor, 1);

uniform vec2 u_dither_size;

uniform vec2 u_dithering_intensity;
uniform vec2 u_dithering_depth;
uniform vec2 u_dithering_color_depth;
uniform vec2 u_dithering_threshold;

void main() {
	vec4 col = texture2D(s_texColor, v_texcoord.xy);
	if(col.a <= 0.0) discard;

	float luma = dot(col.rgb, vec3(0.21267290f, 0.71515220f, 0.07217500f));

	if(u_dither_size.x != 0.0) { // FAST MODE
		vec4 ditherTex = texture2D(s_ditherColor, v_texcoord.xy * u_dither_size.x * u_viewRect.zw);
		float dither = (ditherTex.a - 0.5) * 2.0 / u_dithering_threshold.x;

		col.rgb *= 1.0f + (luma < dither ? (1.0 - luma) * (1.0 - (u_dithering_color_depth.x / 24.0)) * u_dithering_intensity.x * 10.0 : 0.0) * (1.0 / u_dithering_depth.x);
	} else {
		col.rgb = getDither(v_texcoord.xy * u_viewRect.zw, col.rgb, u_dithering_intensity.x * u_dithering_depth.x * 1.0 / 3.0);
	}

	col.rgb = saturate(floor(col.rgb * u_dithering_color_depth.x) / u_dithering_color_depth.x);
	gl_FragColor = col;
}
