$input v_texcoord0

#include <bgfx_shader.sh>
#include "../../include/g-buffer-params.sh"

SAMPLER2D(s_depth, 0);

// from assao sample, cs_assao_prepare_depths.sc
float ScreenSpaceToViewSpaceDepth( float screenDepth ) {
	float depthLinearizeMul = u_depthUnpackConsts.x;
	float depthLinearizeAdd = u_depthUnpackConsts.y;
	return depthLinearizeMul / ( depthLinearizeAdd - screenDepth );
}

void main() {
	float depth = texture2D(s_depth, v_texcoord0).x;
	float linearDepth = ScreenSpaceToViewSpaceDepth(depth);

	gl_FragColor = vec4_splat(linearDepth);
}
