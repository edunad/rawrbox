
#define WRITE_CLUSTER_DATA_GRID
#include <cluster.fxh>

[numthreads(1, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    if(dispatchThreadId.x == 0) {
        g_ClusterDataGrid[0] = 0;
    }
}
