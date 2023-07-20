$input v_color0, v_texcoord0, v_model_0, v_model_1, v_model_2, v_model_3

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/shaderlib.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform vec4 u_decalSettings;


SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_depth, SAMPLE_DEPTH);

mat4 inverse(mat4 m) {
  float
      a00 = m[0][0], a01 = m[0][1], a02 = m[0][2], a03 = m[0][3],
      a10 = m[1][0], a11 = m[1][1], a12 = m[1][2], a13 = m[1][3],
      a20 = m[2][0], a21 = m[2][1], a22 = m[2][2], a23 = m[2][3],
      a30 = m[3][0], a31 = m[3][1], a32 = m[3][2], a33 = m[3][3],

      b00 = a00 * a11 - a01 * a10,
      b01 = a00 * a12 - a02 * a10,
      b02 = a00 * a13 - a03 * a10,
      b03 = a01 * a12 - a02 * a11,
      b04 = a01 * a13 - a03 * a11,
      b05 = a02 * a13 - a03 * a12,
      b06 = a20 * a31 - a21 * a30,
      b07 = a20 * a32 - a22 * a30,
      b08 = a20 * a33 - a23 * a30,
      b09 = a21 * a32 - a22 * a31,
      b10 = a21 * a33 - a23 * a31,
      b11 = a22 * a33 - a23 * a32,

      det = b00 * b11 - b01 * b10 + b02 * b09 + b03 * b08 - b04 * b07 + b05 * b06;

  return mat4(
      a11 * b11 - a12 * b10 + a13 * b09,
      a02 * b10 - a01 * b11 - a03 * b09,
      a31 * b05 - a32 * b04 + a33 * b03,
      a22 * b04 - a21 * b05 - a23 * b03,
      a12 * b08 - a10 * b11 - a13 * b07,
      a00 * b11 - a02 * b08 + a03 * b07,
      a32 * b02 - a30 * b05 - a33 * b01,
      a20 * b05 - a22 * b02 + a23 * b01,
      a10 * b10 - a11 * b08 + a13 * b06,
      a01 * b08 - a00 * b10 - a03 * b06,
      a30 * b04 - a31 * b02 + a33 * b00,
      a21 * b02 - a20 * b04 - a23 * b00,
      a11 * b07 - a10 * b09 - a12 * b06,
      a00 * b09 - a01 * b07 + a02 * b06,
      a31 * b01 - a30 * b03 - a32 * b00,
      a20 * b03 - a21 * b01 + a22 * b00) / det;
}

vec3 reconstructWorldPosition(vec2 texCoord, float rawDepth) {
	vec3 clip = vec3(texCoord * 2.0 - 1.0, toClipSpaceDepth(rawDepth));
	#if BGFX_SHADER_LANGUAGE_HLSL || BGFX_SHADER_LANGUAGE_PSSL || BGFX_SHADER_LANGUAGE_METAL
		clip.y = -clip.y;
	#endif
	vec4 wpos = mul(u_invViewProj, vec4(clip, 1.0));
	return wpos.xyz / wpos.w;
}

void main() {
#if BGFX_SHADER_LANGUAGE_HLSL && (BGFX_SHADER_LANGUAGE_HLSL < 400)
	vec2 screenPosition = gl_FragCoord.xy * u_viewTexel.xy + u_viewTexel.xy * vec2_splat(0.5);
#else
	vec2 screenPosition = gl_FragCoord.xy * u_viewTexel.xy;
#endif

	float rawDepth = texture2D(s_depth, screenPosition).r;
	vec3 worldPosition = reconstructWorldPosition(screenPosition, rawDepth);

	mat4 model = mtxFromCols(v_model_0, v_model_1, v_model_2, v_model_3);

	vec4 objectPosition = mul(inverse(model), vec4(worldPosition, 1.0));
	if(any(greaterThan(abs(objectPosition.xyz), vec3_splat(0.5)))) {
        if(u_decalSettings.x == 0.0) discard;
        gl_FragColor = vec4(0.8, 0.0, 0.0, 0.25);
        return;
    }else if(u_decalSettings.x == 1.0) { // Show debug
        gl_FragColor = vec4(0.8, 0.8, 0.0, 1.0);
        return;
    }

	vec2 decalTexCoord = vec2(0.5 + objectPosition.y, objectPosition.x - 0.5);
	decalTexCoord = objectPosition.xy + 0.5;
    decalTexCoord.xy = 1.0 - decalTexCoord.xy; // Flip textures

    vec4 albedo = texture2DArray(s_albedo, vec3(decalTexCoord.xy, v_texcoord0.z));
    if(albedo.a <= 0.0) discard;

    gl_FragColor = albedo * v_color0;
}
