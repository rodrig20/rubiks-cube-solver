#include "Camera.hpp"

#include <iostream>

#include "esp_camera.h"

Camera::Camera() {}

Color Camera::get_color_piece(camera_fb_t* fb, int posx, int posy, int size) {
    int piece_padding = size / 3;
    uint16_t* pixels = (uint16_t*)fb->buf;
    uint r_sum = 0;
    uint g_sum = 0;
    uint b_sum = 0;

    int x_start = posx + piece_padding;
    int y_start = posy + piece_padding;

    int x_end = posx + size - piece_padding;
    int y_end = posy + size - piece_padding;

    int stride_px = (fb->len / fb->height) / 2;

    uint area = 0;
    for (int y = y_start; y < y_end; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = y * stride_px + x;
            uint16_t pixel = pixels[idx];

            // RGB565: 5 bits red, 6 bits green, 5 bits blue
            uint8_t r = ((pixel >> 11) & 0b11111) << 3;
            uint8_t g = ((pixel >> 5) & 0b111111) << 2;
            uint8_t b = (pixel & 0b11111) << 3;
            //std::cout << static_cast<int>(r) << "+" ;
            r_sum += static_cast<int>(r);
            g_sum += static_cast<int>(g);
            b_sum += static_cast<int>(b);
            area++;
            if (y < y_start + 2 || y >= y_end - 2 || x < x_start + 2 ||
                x >= x_end - 2) {
                uint16_t new_pixel = 0-1;
                pixels[idx] = new_pixel;
            }
        }
    }
    Color color = {r_sum / area, g_sum / area, b_sum / area};

    return color;
}

Color* Camera::get_color_face(camera_fb_t* fb) {
    int cube_padding = fb->height / 7;
    int size = (fb->height - (2 * cube_padding)) / 3;

    Color* cube_state = new Color[9];
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            cube_state[(i * 3) + j] = get_color_piece(
                fb, cube_padding + (j * size), cube_padding + (i * size), size);
        }
    }

    return cube_state;
}