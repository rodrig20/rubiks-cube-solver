#include "Camera.hpp"

#include <iostream>

#include "esp_camera.h"

uint16_t swap_endian(uint16_t value) { return (value >> 8) | (value << 8); }

// Contrutor
Camera::Camera() { startCamera(); }

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

// Inicializa a camara
void Camera::startCamera() {
    // Configuração da camara para Ai-Thinker
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20000000;
    config.frame_size = FRAMESIZE_128X128;
    config.pixel_format = PIXFORMAT_RGB565;
    config.fb_location = CAMERA_FB_IN_PSRAM;

    config.fb_count = 5;
    config.grab_mode = CAMERA_GRAB_LATEST;
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        printf("Erro ao iniciar a câmara: 0x%x", err);
        return;
    }

    // Configuarar Sensor
    sensor_t* s = esp_camera_sensor_get();

    s->set_agc_gain(s, 0);                    // Desativa ganho automático
    s->set_gainceiling(s, (gainceiling_t)2);  // Garante pouco ganho

    s->set_aec2(s, 0);           // Desativa AEC2
    s->set_exposure_ctrl(s, 0);  // Controlo manual da exposição
    s->set_aec_value(s, 85);     // Define expoisção manualmente

    s->set_brightness(s, 1);  // Reduz brilho
    s->set_saturation(s, 3);  // Aumenta muito a saturação
    s->set_contrast(s, 2);    // Contraste normal

    // Pin do LED
    pinMode(LED_PIN, OUTPUT);
}
