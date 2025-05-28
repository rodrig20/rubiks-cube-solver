#include "Camera.hpp"

#include <iostream>

#include "esp_camera.h"

uint16_t swap_endian(uint16_t value) { return (value >> 8) | (value << 8); }

Camera::Camera() {}

// Faz a conversão de RGB565 para RGB
void Camera::rgb565ToRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b) {
    // RGB565: R = 5 bits, G = 6 bits, B = 5 bits
    r = ((color >> 11) & 0x1F) << 3;  // 5 bits para 8 bits
    g = ((color >> 5) & 0x3F) << 2;   // 6 bits para 8 bits
    b = (color & 0x1F) << 3;          // 5 bits para 8 bits
}

Color Camera::get_color_piece(camera_fb_t* fb, int posx, int posy, int size) {
    int piece_padding = size / 5;
    uint16_t* pixels = (uint16_t*)fb->buf;

    int r_sum = 0;
    int g_sum = 0;
    int b_sum = 0;

    int x_start = posx + piece_padding;
    int y_start = posy + piece_padding;

    int x_end = posx + size - piece_padding;
    int y_end = posy + size - piece_padding;

    int stride_px = (fb->len / fb->height) / 2;
    int area = 0;
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = y * fb->width + x;
            uint16_t pixel = swap_endian(pixels[idx]);

            uint8_t r;
            uint8_t g;
            uint8_t b;
            rgb565ToRGB(pixel, r, g, b);
            r_sum += r;
            g_sum += g;
            b_sum += b;
            area++;
        }
    }
    Color color = {(r_sum / area), (g_sum / area), (b_sum / area)};
    return color;
}

void Camera::draw(camera_fb_t* fb, int posx, int posy, int size) {
    int piece_padding = size / 5;
    uint16_t* pixels = (uint16_t*)fb->buf;

    int x_start = posx + piece_padding;
    int y_start = posy + piece_padding;

    int x_end = posx + size - piece_padding;
    int y_end = posy + size - piece_padding;

    int stride_px = (fb->len / fb->height) / 2;

    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = y * fb->width + x;

            if (y < y_start + 2 || y >= y_end - 2 || x < x_start + 2 ||
                x >= x_end - 2) {
                uint16_t new_pixel = ~0;
                pixels[idx] = new_pixel;
            }
        }
    }
}

// Obtem a lista de cores de cores de uma face
Color* Camera::get_color_face(camera_fb_t* fb) {
    int cube_padding = fb->height / 10;
    int size = (fb->height - (2 * cube_padding)) / 3;
    Color* cube_state = new Color[9];

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cube_state[(i * 3) + j] = get_color_piece(
                fb, cube_padding + (j * size), cube_padding + (i * size), size);
        }
    }

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            draw(fb, cube_padding + (j * size), cube_padding + (i * size),
                 size);
        }
    }

    return cube_state;
}
