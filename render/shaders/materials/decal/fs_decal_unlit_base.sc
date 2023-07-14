$input v_color0, v_texcoord0, v_position, v_id

#include <bgfx_shader.sh>
#include "../../include/defs.sh"
#include "../../include/shaderlib.sh"

uniform vec4 u_colorOffset;
uniform vec4 u_camPos;

uniform mat4 u_invModel; // Inverse model transform matrix.

//https://github.com/lukaspj/Torque6/blob/master/projects/shared-modules/AppCore/1/shaders/gui/decal_fs.tsh

SAMPLER2DARRAY(s_albedo, SAMPLE_MAT_ALBEDO);
SAMPLER2D(s_depth, SAMPLE_DEPTH);

// Space Conversion Functions
vec2 toUVSpace( vec3 screenspacePos ) {
    vec2 outPos = ( screenspacePos.xy + 1.0 ) / 2.0;
#if BGFX_SHADER_LANGUAGE_HLSL
    outPos.y = 1.0 - outPos.y;
#endif
    return outPos;
}

vec2 toClipSpace( vec2 uvPos ){
    vec2 outPos = uvPos;
    outPos = ( outPos.xy * 2.0 ) - 1.0;

#if BGFX_SHADER_LANGUAGE_HLSL
    outPos.y = -outPos.y;
#endif

    return outPos;
}

void main() {
    // Obtain Depth
    vec3 sspos          = v_position.xyz / v_position.w;
    vec2 uv_coords      = toUVSpace( sspos );

    float deviceDepth   = texture2D(s_depth, uv_coords).x;
    float depth         = toClipSpaceDepth(deviceDepth);


    // Reconstruct world space position
    vec3 clip = vec3(sspos.xy, depth);
    vec3 wpos = clipToWorld(u_invViewProj, clip);

    vec4 vsPos = mul(u_view, vec4(wpos, 1.0));
    vsPos.xyz /= vsPos.w;

    // Project into model space
    vec4 modelPos = mul(u_invModel, vec4(wpos.xyz, 1.0));
    modelPos.xyz /= modelPos.w;

    // Clip areas outside of the model.
    // TODO: Base this on actual model scale not fixed size of 2.0
    float size = 2.0;
    if ( modelPos.x < -size || modelPos.y < -size || modelPos.z < -size || modelPos.x > size || modelPos.y > size || modelPos.z > size )
        discard;

    // Adjust model transform to fit UV space better.
    vec2 decalUV = modelPos.xz;
    decalUV -= vec2(1.0, 1.0);
    decalUV /= 2.0;

    vec4 albedo = texture2DArray(s_albedo, vec3(decalUV.xy, v_texcoord0.z)) * v_color0 * u_colorOffset;
	if (albedo.a <= 0.2) discard;

	gl_FragColor = albedo;
}
