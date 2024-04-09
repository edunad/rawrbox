#include "math.fxh"

#define WRITE_CLUSTER_DATA_GRID
#include "cluster.fxh"

[numthreads(CLUSTERS_X_THREADS, CLUSTERS_Y_THREADS, CLUSTERS_Z_THREADS)]
void main(uint3 dispatchThreadId: SV_DispatchThreadID) {
	if (any(dispatchThreadId >= GROUP_SIZE.xyz))
		return;

	uint clusterIndex = Flatten3D(dispatchThreadId, float2(CLUSTERS_X, CLUSTERS_Y));
	ClusterDataGrid[clusterIndex] = (ClusterData)0;
}
