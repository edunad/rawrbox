#pragma once

// ---------------------------------
// DEFINES RELATED TO RAWRBOX.RENDER
// ---------------------------------

// RENDERER ------
// NOTE: IF THIS VALUE IS TOO HIGH, YOU MIGHT NEED TO INCREASE THE HEAP MEMORY
#define RB_RENDER_MAX_TEXTURES 8192
// NOTE: IF THIS VALUE IS TOO HIGH, YOU MIGHT NEED TO INCREASE THE HEAP MEMORY
#define RB_RENDER_MAX_VERTEX_TEXTURES  128
#define RB_RENDER_GPU_PICK_SAMPLE_SIZE 2
// ------

// MODELS ------
#define RB_RENDER_MAX_VERTICES 16000
#define RB_RENDER_MAX_INDICES  RB_RENDER_MAX_VERTICES * 3

#define RB_MAX_BONES_PER_VERTEX          4
#define RB_RENDER_MAX_BONES_PER_MODEL    150
#define RB_RENDER_BUFFER_INCREASE_OFFSET 256 // To prevent the vertex / index buffer from resizing too often, increase this value to offset the scaling based on your model needs
// -------------

// SHADERS ------
#define RB_RENDER_MAX_POST_DATA         2
#define RB_RENDER_RENDER_TARGET_TARGETS 2
// -------------

// CLUSTERED ------
#define RB_RENDER_CLUSTER_TEXTEL_SIZE 64
#define RB_RENDER_CLUSTERS_Z          32

#define RB_RENDER_MAX_DATA_PER_CLUSTER 256

#define RB_RENDER_CLUSTERED_NUM_BUCKETS (RB_RENDER_MAX_DATA_PER_CLUSTER / RB_RENDER_CLUSTERS_Z)

#define RB_RENDER_CLUSTERS_X_THREADS 4
#define RB_RENDER_CLUSTERS_Y_THREADS 4
#define RB_RENDER_CLUSTERS_Z_THREADS 4

#define RB_RENDER_THREAD_GROUP_SIZE (RB_RENDER_CLUSTERS_X_THREADS * RB_RENDER_CLUSTERS_Y_THREADS * RB_RENDER_CLUSTERS_Z_THREADS)
// ----------------

// CAMERA -----
// Add more layers if needed, you can then check if the camera should draw your model by checking if the layer is enabled
#define RB_RENDER_CAMERA_LAYERS(flag) \
	flag(MY_LAYER_1, 1) \
	    flag(MY_LAYER_2, 2)
// ----------------
