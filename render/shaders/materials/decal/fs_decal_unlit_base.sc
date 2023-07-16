$input v_color0, v_texcoord0, v_wpos

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/shaderlib.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform mat4 u_decalMatrix;

//https://github.com/lukaspj/Torque6/blob/master/projects/shared-modules/AppCore/1/shaders/gui/decal_fs.tsh

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_depth, SAMPLE_DEPTH);

vec3 getScreenCoord(vec3 world_pos)
{
	vec4 prj = mul(u_viewProj, vec4(world_pos, 1.0) );
	prj.y = -prj.y;
	prj /= prj.w;
	return prj.xyz;
}

vec3 getViewPosition(sampler2D depth_buffer, mat4 inv_view_proj, vec2 tex_coord)
{
	float z = texture2D(depth_buffer, tex_coord).r;
	#if BGFX_SHADER_LANGUAGE_HLSL
		z = z;
	#else
		z = z * 2.0 - 1.0;
	#endif // BGFX_SHADER_LANGUAGE_HLSL

	vec4 pos_proj = vec4(tex_coord * 2.0 - 1.0, z, 1.0);
	#if BGFX_SHADER_LANGUAGE_HLSL
		pos_proj.y = -pos_proj.y;
	#endif // BGFX_SHADER_LANGUAGE_HLSL

	vec4 view_pos = mul(inv_view_proj, pos_proj);

	return view_pos.xyz / view_pos.w;
}

// https://martindevans.me/game-development/2015/02/27/Drawing-Stuff-On-Other-Stuff-With-Deferred-Screenspace-Decals/
void main() {
#if BGFX_SHADER_LANGUAGE_HLSL && (BGFX_SHADER_LANGUAGE_HLSL < 400)
	vec2 texCoord = gl_FragCoord.xy * u_viewTexel.xy + u_viewTexel.xy * vec2_splat(0.5);
#else
	vec2 texCoord = gl_FragCoord.xy * u_viewTexel.xy;
#endif

	// Get world position
	float deviceDepth = texture2D(s_depth, texCoord).x;
	float depth       = toClipSpaceDepth(deviceDepth);

	vec3 clip = vec3(texCoord * 2.0 - 1.0, depth);
#if !BGFX_SHADER_LANGUAGE_GLSL
	clip.y = -clip.y;
#endif // !BGFX_SHADER_LANGUAGE_GLSL

	vec3 wpos = clipToWorld(u_invViewProj, clip);




/*



	vec3 screen_coord = getScreenCoord(v_wpos);
	vec3 wpos = getViewPosition(s_depth, u_invViewProj, screen_coord.xy * 0.5 + 0.5);

	vec3 tmp = mul(u_decalMatrix, vec4(wpos, 1)).xyz;
	if(any(greaterThan(abs(tmp.xyz), vec3_splat(1.0)))) discard;

	gl_FragColor = vec4(1.0,0.0,0.0,1.0);

	vec3 tmp = mul(u_decalMatrix, vec4(wpos, 1)).xyz;
	if(any(greaterThan(abs(tmp.xyz), vec3_splat(1.0)))) discard;
*/
	gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}

    // Clip areas outside of the model.
    // TODO: Base this on actual model scale not fixed size of 2.0
   /* float size = 4.0;
    if ( modelPos.x < -size || modelPos.y < -size || modelPos.z < -size || modelPos.x > size || modelPos.y > size || modelPos.z > size )
        discard;

    // Adjust model transform to fit UV space better.
    vec2 decalUV = modelPos.xz;
    decalUV -= vec2(1.0, 1.0);
    decalUV /= 2.0;

    vec4 albedo = texture2DArray(s_albedo, vec3(decalUV.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.r >= 0.85 && albedo.g >= 0.85 && albedo.b >= 0.85) discard;

	gl_FragColor = albedo;
}
*/
