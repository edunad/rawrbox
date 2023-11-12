#define WRITE_ATOMIC
#include <cluster.fxh>

[numthreads(1, 1, 1)]
void main(uint3 GIid: SV_DispatchThreadID) {
    if(GIid.x == 0) {
        // reset the atomic counter for the light grid generation
        // writable compute buffers can't be updated by CPU so do it here
        g_globalIndex[0] = 0;
    }
}
