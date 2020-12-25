#pragma once
#include "includes.hpp"

constexpr rgba_t black = { 0x00, 0x00, 0x00, 0xFF };
constexpr rgba_t white = { 0xFF, 0xFF, 0xFF, 0xFF };
constexpr rgba_t gray  = { 0x80, 0x80, 0x80, 0xFF };
constexpr rgba_t red   = { 0xFF, 0x00, 0x00, 0xFF };
constexpr rgba_t green = { 0x00, 0xFF, 0x00, 0xFF };

class image_manip { //class for manipulating opengl textures
	GLuint *m_texture_id = nullptr;
	uint8_t* m_texture_buffer = nullptr;
	int* m_width = nullptr, *m_height = nullptr;
	const static int m_channels = 4;
	
	void lock_texture() {
		m_texture_buffer = new uint8_t[*m_width * *m_height * m_channels]();
		GLuint fbo;
		glGenFramebuffers(1, &fbo); 
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, *m_texture_id, 0);
		
		glReadPixels(0, 0, *m_width, *m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_texture_buffer);
		
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	void unlock_texture() {
		glBindTexture(GL_TEXTURE_2D, *m_texture_id);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, *m_width, *m_height, GL_RGBA, GL_UNSIGNED_BYTE, m_texture_buffer);
		glBindTexture(GL_TEXTURE_2D, 0);
		delete[] m_texture_buffer;
	}

	rgba_t* get_pixel_ptr(const point_t point) {
		return (rgba_t*)((uintptr_t)m_texture_buffer + (point.y * (*m_width * m_channels)) + point.x * m_channels);
	}

	rgba_t get_pixel(const point_t point) {
		return *get_pixel_ptr(point);
	}

	void set_pixel(const point_t point, const rgba_t color) {
		*get_pixel_ptr(point) = color;
	}

	uint8_t get_luminance(const point_t point) {
		const auto cols = get_pixel(point);
		return uint8_t(0.2126f * cols.r + 0.7152f * cols.g + 0.0722f * cols.b); //relative luminance = 0.2126R + 0.7152G + 0.0722B
	}
	
public:
	image_manip(GLuint *ptexture_id, int* width, int* height) : m_texture_id(ptexture_id), m_width(width), m_height(height) { }

	void binarize_texture(const int threshold = 200) {
		lock_texture();
		for (int x = 0; x < *m_width; x++)
			for (int y = 0; y < *m_height; y++)
				get_luminance({ x, y }) > threshold ? set_pixel({ x, y }, white) : set_pixel({ x, y }, black);
		unlock_texture();
	}

	void darken_background() {
		lock_texture();
		for (int y = 0; y < *m_height; y++)
			for (int x = 0; x < *m_width; x++)
				if (get_pixel({ x, y }) == white)
					set_pixel({ x, y }, gray);
		unlock_texture();
	}

	auto get_texture_as_bool_vector() {
		lock_texture();
		std::vector<bool> out;
		for (int y = 0; y < *m_height; y++)
			for (int x = 0; x < *m_width; x++)
				out.push_back(get_pixel({ x, y }) == white);
		return out;
		unlock_texture();
	}

	void draw_points(const std::vector<std::tuple<int, int, rgba_t>>& pixels) {
		lock_texture();
		for (const auto& pixel : pixels)
			set_pixel({ std::get<0>(pixel), std::get<1>(pixel) }, std::get<2>(pixel));
		unlock_texture();
	}

	void draw_markers(const point_t start, const point_t end, int marker_size) {
		static std::vector<std::tuple<int, int, rgba_t>> storage;
		static auto cached_texture = *m_texture_id;
		if (*m_texture_id != cached_texture) {
			storage.clear();
			cached_texture = *m_texture_id;
		}
		lock_texture();
		for (const auto& pix : storage) {
			if (std::get<0>(pix) > *m_width - 1 || std::get<1>(pix) > *m_height - 1)
				continue;
			set_pixel({ std::get<0>(pix), std::get<1>(pix) }, std::get<2>(pix)); //put back the overwritten pixels
		}
		storage.clear();
		for (int y = 0; y < marker_size; y++) {
			for (int x = 0; x < marker_size; x++) {
				if (start.x + x > *m_width - 1 || start.y + y > *m_height - 1) continue;
				storage.push_back({ start.x + x, start.y + y, get_pixel({start.x + x, start.y + y}) }); //save pixels overwritten by start marker
			}
		}
		for (int y = 0; y < marker_size; y++) {
			for (int x = 0; x < marker_size; x++) {
				if (end.x + x > * m_width - 1 || end.y + y > * m_height - 1) continue;
				storage.push_back({ end.x + x, end.y + y, get_pixel({end.x + x, end.y + y}) }); //save pixels overwritten by end marker
			}
		}
		for (int y = 0; y < marker_size; y++) {
			for (int x = 0; x < marker_size; x++) {
				if (start.x + x > *m_width - 1 || start.y + y > *m_height - 1) continue;
				set_pixel({ start.x + x, start.y + y }, red); //draw start marker
			}
		}
		for (int y = 0; y < marker_size; y++) {
			for (int x = 0; x < marker_size; x++) {
				if (end.x + x > * m_width - 1 || end.y + y > * m_height - 1) continue;
				set_pixel({ end.x + x, end.y + y }, green); //draw end marker
			}
		}
		unlock_texture();
	}

	std::vector<rgba_t> get_image_data() {
		lock_texture();
		std::vector<rgba_t> vec(*m_width * *m_height);
		memcpy(vec.data(), m_texture_buffer, *m_width * *m_height * m_channels);
		unlock_texture();
		return vec;
	}
};