
$input v_texcoord, v_gpuPick, v_color0, v_worldPos, v_data

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/model_transforms.sh"
#include "../../include/fog.sh"

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

void main() {
	vec4 albedo = texture2DArray(s_albedo, vec3(v_texcoord.xy, v_data.x)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.0) discard;

	gl_FragData[0] = albedo; // COLOR

	// Apply Fog ----
	gl_FragData[0] = applyFog(gl_FragData[0], v_worldPos, u_camPos); // TODO: FIX ME
	// -------

	gl_FragData[1].r = 1.F - recieve_decals; // DECALS

 	// GPU PICKING -----
	bool alpha = v_gpuPick.r == 0. && v_gpuPick.g == 0. && v_gpuPick.b == 0.;
	gl_FragData[2].rgba = vec4(v_gpuPick.rgb, alpha ? 0. : 1.);
	// -----------------
}
