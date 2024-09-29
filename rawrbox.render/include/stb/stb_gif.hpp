// From: https://gist.github.com/urraka/685d9a6340b26b830d49
#pragma once

#pragma warning(push)
#pragma warning(disable : 4505)
#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_ONLY_TGA

#define STBI_FREE(ptr)           std::free(ptr)
#define STBI_MALLOC(size)        std::malloc(size)
#define STBI_REALLOC(ptr, nsize) std::realloc(ptr, nsize)

#include <stb/stb_image.hpp>
#include <stb/stb_image_write.hpp>
#pragma warning(pop)

#include <stdexcept>

// NOLINTBEGIN(*)
const int defaultFrameDelayCentiseconds = 4;

using gif_result = struct gif_result_t {
	int delay;
	uint8_t* data;
	struct gif_result_t* next;
};

uint8_t* stbi_xload(stbi__context* s, int* x, int* y, int* frames, int** delays) {
	int comp = 0;
	return reinterpret_cast<uint8_t*>(stbi__load_gif_main(s, delays, x, y, frames, &comp, 4)); // Force 4 channels
}

uint8_t* stbi_xload_file(char const* filename, int* x, int* y, int* frames, int** delays) {
	stbi__context s;

	FILE* f = stbi__fopen(filename, "rb");
	if (f == nullptr) throw std::runtime_error("Failed to open file");
	stbi__start_file(&s, f);

	if (!stbi__gif_test(&s)) {
		fclose(f);
		throw std::runtime_error("Given file not a GIF");
	}

	auto ret = stbi_xload(&s, x, y, frames, delays);
	fclose(f);

	return ret;
};

uint8_t* stbi_xload_mem(const uint8_t* buffer, int len, int* x, int* y, int* frames, int** delays) {
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi_xload(&s, x, y, frames, delays);
};
// NOLINTEND(*)
