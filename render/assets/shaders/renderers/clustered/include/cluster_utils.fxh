#ifndef INCLUDED_CLUSTER_UTILS
#define INCLUDED_CLUSTER_UTILS

#include <utils.fxh>

// cluster depth index from depth in screen coordinates (gl_FragCoord.z)
uint getClusterZIndex(float screenDepth) {

    // this can be calculated on the CPU and passed as a uniform
    // only leaving it here to keep most of the relevant code in the shaders for learning purposes
    float scale = float(CLUSTERS_Z) / log(g_zNearFarVec.y / g_zNearFarVec.x);
    float bias = -(float(CLUSTERS_Z) * log(g_zNearFarVec.x) / log(g_zNearFarVec.y / g_zNearFarVec.x));

    float eyeDepth = screen2EyeDepth(screenDepth, g_zNearFarVec.x, g_zNearFarVec.y);
    uint zIndex = uint(max(log(eyeDepth) * scale + bias, 0.0));

    return zIndex;
}

#endif
