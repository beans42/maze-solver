#pragma once
#include <emscripten.h>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define GLFW_INCLUDE_ES3
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb/stb_image_write.h"

#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <tuple>
#include <stack>

EM_JS(int, canvas_get_width, (), { return Module.canvas.width; });
EM_JS(int, canvas_get_height, (), { return Module.canvas.height; });
EM_JS(void, resize_canvas, (), { js_resize_canvas(); });
EM_JS(void, file_dialog, (), { js_file_dialog(); });
EM_JS(void, alert_popup, (const char* string, size_t length), { js_alert_popup(string, length); });
EM_JS(void, open_url, (const char* string, size_t length), { js_open_url(string, length); });
EM_JS(void, open_data_url, (const char* string, size_t length), { js_open_data_url(string, length); });

template<int length>
void alert(const char (&string)[length]) { alert_popup(string, length - 1); }

struct point_t {
	int x, y;
	bool operator==(const point_t& other) const { return std::tie(x, y) == std::tie(other.x, other.y); }
	auto neighbours(const int width, const int height) const {
		std::vector<point_t> out;
		if (x + 1 < width)  out.push_back({ x + 1, y });
		if (y + 1 < height) out.push_back({ x, y + 1 });
		if (x - 1 >= 0) out.push_back({ x - 1, y });
		if (y - 1 >= 0) out.push_back({ x, y - 1 });
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

bool load_texture_from_file(const uint8_t* buffer, const size_t size, GLuint* out_texture, unsigned* out_width, unsigned* out_height) {
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	int image_width = 0, image_height = 0;
	unsigned char* image_data = stbi_load_from_memory(buffer, size, &image_width, &image_height, NULL, 4);
	if (image_data == NULL) return false;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;
	return true;
}

std::string base64_encode(const char* data, const size_t in_len) {
	constexpr char encoding_table[] = {
		'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
		'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
		'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
		'w', 'x', 'y', 'z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'
	};
	const size_t out_len = 4 * ((in_len + 2) / 3);
	std::string ret(out_len, '\0');
	auto p = (char*)ret.data();
	size_t i;
	for (i = 0; i < in_len - 2; i += 3) {
		*p++ = encoding_table[(data[i] >> 2) & 0x3F];
		*p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
		*p++ = encoding_table[((data[i + 1] & 0xF) << 2) | ((int) (data[i + 2] & 0xC0) >> 6)];
		*p++ = encoding_table[data[i + 2] & 0x3F];
	}
	if (i < in_len) {
		*p++ = encoding_table[(data[i] >> 2) & 0x3F];
		if (i == (in_len - 1)) {
			*p++ = encoding_table[((data[i] & 0x3) << 4)];
			*p++ = '=';
		} else {
			*p++ = encoding_table[((data[i] & 0x3) << 4) | ((int) (data[i + 1] & 0xF0) >> 4)];
			*p++ = encoding_table[((data[i + 1] & 0xF) << 2)];
		}
		*p++ = '=';
	}
	return ret;
}