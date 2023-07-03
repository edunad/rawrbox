$input v_texcoord0

/*
* Copyright 2021 elven cache. All rights reserved.
* License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
*/

#include <bgfx_shader.sh>
#include "../../include/shaderlib.sh"

// Lighting
uniform vec4 u_lightPosition;
uniform mat4 u_lightData;
//uniform mat4 u_mtx;
// -----

SAMPLER2D(s_normal, 0);
SAMPLER2D(s_depth, 1);

vec2 blinn(vec3 _lightDir, vec3 _normal, vec3 _viewDir) {
	float ndotl = dot(_normal, _lightDir);
	vec3 reflected = _lightDir - 2.0 * ndotl * _normal;
	float rdotv = dot(reflected, _viewDir);
	return vec2(ndotl, rdotv);
}

vec4 lit(float _ndotl, float _rdotv, float _m) {
	float diff = max(0.0, _ndotl);
	float spec = step(0.0, _ndotl) * max(0.0, _rdotv * _m);
	return vec4(1.0, diff, spec, 1.0);
}

vec3 calculatePointLight(vec3 wpos, vec3 view, vec3 normal) {
	vec3 lightPos = normalize(u_lightPosition.xyz - wpos);

	float _lightInner = 0.8;
	float attn = 1.0 - smoothstep(_lightInner, 1.0, length(lightPos) / u_lightPosition.w);

	vec3 lightDir = normalize(lightPos);
	vec2 bln = blinn(lightDir, normal, view);
	vec4 lc = lit(bln.x, bln.y, 1.0);

	return vec3(u_lightData[0][0], u_lightData[0][1], u_lightData[0][2]) * lc.y * attn;
}

void main() {
	/*vec3  position    = texture2D(s_position, v_texcoord0).rgb;
	vec3  normal      = decodeNormalUint(texture2D(s_normal, v_texcoord0).xyz);

	vec3 view = mul(u_view, vec4(position, 0.0)).xyz;
	view = -normalize(view);

	vec3 light = vec3_splat(0.);
	if(u_lightData[3][0] == 1.0) { // POINT
		light = calculatePointLight(position, view, normal);
	}else if(u_lightData[3][0] == 2.0) { // SPOT
		//gl_FragColor = vec4(calculateSpotLight(i, v_view, normal, texColor, specularColor);
	}else if(u_lightData[3][0] == 3.0) { // DIR
		//gl_FragColor = vec4(calculateDirectionalLight(i, v_view, normal, texColor, specularColor);
	}

	gl_FragColor = vec4(light, 1.);*/
}
