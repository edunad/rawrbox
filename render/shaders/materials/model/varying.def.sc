vec3 a_position         : POSITION;
vec4 a_texcoord0        : TEXCOORD0;
vec4 a_color0           : COLOR0;
vec4 a_color1           : COLOR1;
vec4 a_normal           : NORMAL;
vec4 a_tangent          : TANGENT;
ivec4 a_indices         : BLENDINDICES0;
vec4 a_weight           : BLENDWEIGHT0;

vec3 v_texcoord  : TEXCOORD0 = vec3(0.0, 0.0, 0.0);
flat vec3 v_data : TEXCOORD1 = vec3(0.0, 0.0, 0.0);
vec3 v_worldPos  : POSITION1 = vec3(0.0, 0.0, 0.0);
vec3 v_normal    : NORMAL    = vec3(0.0, 0.0, 1.0);
vec3 v_tangent   : TANGENT   = vec3(0.0, 0.0, 1.0);
vec4 v_color0    : COLOR0    = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_gpuPick   : COLOR1    = vec4(0.0, 0.0, 0.0, 0.0);
