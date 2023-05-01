// From: https://gist.github.com/urraka/685d9a6340b26b830d49
#pragma once

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#define STBI_ONLY_PNG
#define STBI_ONLY_JPEG
#define STBI_ONLY_BMP
#define STBI_ONLY_GIF
#define STBI_ONLY_TGA

#include <stb/image.hpp>
#include <stb/image_write.hpp>

#include <stdexcept>
#include <vector>

// NOLINTBEGIN(*)
const int defaultFrameDelayCentiseconds = 4;

using gif_result = struct gif_result_t {
	int delay;
	unsigned char *data;
	struct gif_result_t *next;
};

unsigned char *stbi_xload(char const *filename, int *x, int *y, int *frames, int **delays) {
	FILE *f = nullptr;

	int comp = 0;
	unsigned char *result = nullptr;
	stbi__context s;

	if (!(f = stbi__fopen(filename, "rb"))) throw std::runtime_error("Failed to open file");
	stbi__start_file(&s, f);

	if (!stbi__gif_test(&s)) {
		fclose(f);
		throw std::runtime_error("Give file not a GIF");
	}

	auto ret = reinterpret_cast<unsigned char *>(stbi__load_gif_main(&s, delays, x, y, frames, &comp, 4));
	fclose(f);

	return ret;
};
// NOLINTEND(*)
