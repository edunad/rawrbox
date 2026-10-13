Texture2DArray<float> Depth;
RWTexture2D<float> Source;
RWTexture2D<float> Target;

// https://docs.mesa3d.org/isl/hiz.html
// https://www.rastergrid.com/blog/2010/10/hierarchical-z-map-based-occlusion-culling/

// BLOCK 2x2 ---
int4 GetBlock(uint2 id, uint2 sourceSize, uint2 targetSize) {
	int2 first = int2(id) * 2;
	int2 last = first + 1;

	if (id.x == targetSize.x - 1) last.x = int(sourceSize.x) - 1;
	if (id.y == targetSize.y - 1) last.y = int(sourceSize.y) - 1;

	return int4(first, min(last, int2(sourceSize) - 1));
}
// ------

[numthreads(REDUCE_THREADS, REDUCE_THREADS, 1)]
void DepthReduce(uint3 id : SV_DispatchThreadID) {
	uint2 targetSize;
	Target.GetDimensions(targetSize.x, targetSize.y);
	if (any(id.xy >= targetSize)) return;

	uint3 sourceSize;
	Depth.GetDimensions(sourceSize.x, sourceSize.y, sourceSize.z);

	int4 block = GetBlock(id.xy, sourceSize.xy, targetSize);
	float far = 0.0;

	for (int y = block.y; y <= block.w; y++) {
		for (int x = block.x; x <= block.z; x++) {
			far = max(far, Depth.Load(int4(x, y, 0, 0)));
		}
	}

	Target[id.xy] = far;
}

[numthreads(REDUCE_THREADS, REDUCE_THREADS, 1)]
void Reduce(uint3 id : SV_DispatchThreadID) {
	uint2 targetSize;
	Target.GetDimensions(targetSize.x, targetSize.y);

	if (any(id.xy >= targetSize)) return;

	uint2 sourceSize;
	Source.GetDimensions(sourceSize.x, sourceSize.y);

	int4 block = GetBlock(id.xy, sourceSize, targetSize);
	float far = 0.0;

	for (int y = block.y; y <= block.w; y++) {
		for (int x = block.x; x <= block.z; x++) {
			far = max(far, Source[int2(x, y)]);
		}
	}

	Target[id.xy] = far;
}