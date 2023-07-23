vec3 a_position    : POSITION;
vec4 a_texcoord0   : TEXCOORD0;
vec4 a_color0      : COLOR0;
vec4 a_normal      : NORMAL;
vec4 a_tangent     : TANGENT;

vec3 v_texcoord0   : TEXCOORD0 = vec3(0.0, 0.0, 0.0);
vec4 v_color0      : COLOR0    = vec4(0.0, 0.0, 0.0, 0.0);
vec3 v_normal      : NORMAL    = vec3(0.0, 0.0, 1.0);
vec3 v_tangent     : TANGENT   = vec3(0.0, 0.0, 1.0);
vec4 v_model_0     : POSITION1 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_model_1     : POSITION2 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_model_2     : POSITION3 = vec4(0.0, 0.0, 0.0, 0.0);
vec4 v_model_3     : POSITION4 = vec4(0.0, 0.0, 0.0, 0.0);
