
#define WRITE_CLUSTER_DATA_GRID
#include "cluster.fxh"

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint3 dispatchThreadId : SV_DispatchThreadID) {
    if(any(dispatchThreadId >= GROUP_SIZE.xyz))
		return;

	uint clusterIndex = Flatten3D(dispatchThreadId, float2(CLUSTERS_X, CLUSTERS_Y));
    ClusterDataGrid[clusterIndex] = 0;
}
