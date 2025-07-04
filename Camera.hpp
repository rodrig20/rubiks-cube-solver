#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <Arduino.h>

#include "esp_camera.h"

// Pinos da câmara (modelo AI-Thinker ESP32-CAM com OV2640)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define LED_PIN 4
#define LED_BRIGHTNESS 22
#define LED_BRIGHTNESS_DELAY 50

typedef struct {
    uint8_t R;
    uint8_t G;
    uint8_t B;
} Color;

typedef struct {
    float H;  // 0-360
    float S;  // 0-1
    float V;  // 0-1
} HSV;

class Camera {
   public:
    int initialized = 0;
    Camera();
    void rgb565ToRGB(uint16_t color, uint8_t& r, uint8_t& g, uint8_t& b);
    Color get_color_piece(camera_fb_t* fb, int posx, int posy, int size);
    Color* get_color_face();
    Color* get_color_face_debug(camera_fb_t* fb);
    static void draw(camera_fb_t* fb, int posx, int posy, int size);
    void startCamera();
    void grouping_colors(Color cores[54], int labels[54]);
};

#endif