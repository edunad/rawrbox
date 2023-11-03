#ifndef INCLUDED_CLUSTER_UTILS
#define INCLUDED_CLUSTER_UTILS

#include <utils.fxh>

// cluster depth index from depth in screen coordinates (gl_FragCoord.z)
uint getClusterZIndex(float screenDepth) {

    // this can be calculated on the CPU and passed as a uniform
    // only leaving it here to keep most of the relevant code in the shaders for learning purposes
    float scale = float(CLUSTERS_Z) / log(u_zFar / u_zNear);
    float bias = -(float(CLUSTERS_Z) * log(u_zNear) / log(u_zFar / u_zNear));

    float eyeDepth = screen2EyeDepth(screenDepth, u_zNear, u_zFar);
    uint zIndex = uint(max(log(eyeDepth) * scale + bias, 0.0));

    return zIndex;
}

#endif
