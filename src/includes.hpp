#pragma once
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

#include "../tinyfiledialogs/tinyfiledialogs.h"

#include <algorithm>
#include <string>
#include <thread>
#include <vector>
#include <queue>
#include <stack>

struct point_t {
	int x, y;
	bool operator==(const point_t& other) const { return std::tie(x, y) == std::tie(other.x, other.y); }
	auto neighbours(const int width, const int height) const {
		std::vector<point_t> out;
#if 1
		if (x + 1 < width)  out.push_back({ x + 1, y });
		if (y + 1 < height) out.push_back({ x, y + 1 });
		if (x - 1 >= 0) out.push_back({ x - 1, y });
		if (y - 1 >= 0) out.push_back({ x, y - 1 });

#else
		for (int y_offset = -1; y_offset <= 1; ++y_offset) {
			for (int x_offset = -1; x_offset <= 1; ++x_offset) {
				if (x_offset == 0 && y_offset == 0) continue;
				if (x + x_offset < 0 || y + y_offset < 0) continue;
				if (x + x_offset >= width || y + y_offset >= height) continue;
				out.push_back({ x + x_offset, y + y_offset });
			}
		}
#endif
		return out;
	}
};

struct maze_t {
	unsigned width, height;
	point_t start, end;
	std::vector<bool> grid;
};

struct ret_t {
	bool solved;
	std::vector<unsigned> cost_map;
	std::vector<point_t> path;
};

struct rgba_t {
	uint8_t r, g, b, a;
	bool operator==(const rgba_t& other) const {
		return std::tie(r, g, b, a) == std::tie(other.r, other.g, other.b, other.a);
	}
};

struct solution_interface { virtual ret_t solve(const maze_t& maze) = 0; };

std::string get_file_name() {
	const char* const file_types[] = { "*.bmp", "*.dds", "*.dib", "*.hdr", "*.jpg", "*.pfm", "*.png", "*.ppm", "*.tga" };
	const auto ret = tinyfd_openFileDialog("", "", sizeof(file_types) / sizeof(file_types[0]), file_types, "images", 0);
	return ret ? std::string(ret) : "";
}

bool load_texture_from_file(const char* filename, GLuint* out_texture, unsigned* out_width, unsigned* out_height) {
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	int image_width = 0, image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL) return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;
	return true;
}
